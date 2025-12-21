/*
 * uefi_network.cpp
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include "network.hpp"
#include "../boot/libb/include/bloodhorn/uefi.h"
#include <efilib.h>
#include <efiprot.h>
#include <efipciio.h>
#include "arp.h"
#include "dhcp.h"
#include "tftp.h"
#include "pxe.h"

namespace BloodHorn::Net {

class UefiNetworkInterface : public NetworkInterface {
public:
    UefiNetworkInterface() = default;
    
    std::error_code initialize() override {
        EFI_STATUS status = EFI_SUCCESS;
        
        // Initialize network protocols
        status = gBS->LocateProtocol(&gEfiSimpleNetworkProtocolGuid, nullptr, (void**)&snp_);
        if (EFI_ERROR(status)) {
            return std::make_error_code(std::errc::io_error);
        }
        
        // Start the network interface
        status = snp_->Start(snp_);
        if (EFI_ERROR(status)) {
            return std::make_error_code(std::errc::io_error);
        }
        
        // Get MAC address
        mac_ = MacAddress(snp_->Mode->CurrentAddress.Addr);
        
        return {};
    }
    
    std::error_code shutdown() override {
        if (snp_) {
            snp_->Stop(snp_);
            snp_ = nullptr;
        }
        return {};
    }
    
    std::error_code resolveIpToMac(const IPv4Address& ip, MacAddress& mac) override {
        uint8_t mac_addr[6];
        int result = arp_resolve(ip.octets().data(), mac_addr);
        if (result != 0) {
            return std::make_error_code(std::errc::host_unreachable);
        }
        mac = MacAddress(mac_addr);
        return {};
    }
    
    std::error_code pxeDiscover(NetworkConfig& config) override {
        struct pxe_network_info* net_info = pxe_get_network_info();
        if (!net_info) {
            return std::make_error_code(std::errc::network_unreachable);
        }
        
        // Fill in the configuration
        config.ip_address = IPv4Address(net_info->client_ip);
        config.netmask = IPv4Address(net_info->subnet_mask);
        config.gateway = IPv4Address(net_info->router_ip);
        config.dns_server = IPv4Address(net_info->dns_server);
        config.tftp_server = net_info->tftp_server;
        config.boot_file = net_info->boot_file;
        config.domain_name = net_info->domain_name;
        
        // Get MAC address from SNP
        if (snp_) {
            config.mac_address = MacAddress(snp_->Mode->CurrentAddress.Addr);
        }
        
        return {};
    }
    
    std::error_code tftpDownload(const std::string& server, const std::string& remote_path,
                               std::vector<uint8_t>& data) override {
        uint8_t* file_data = nullptr;
        uint32_t file_size = 0;
        
        int result = tftp_get_file(server.c_str(), remote_path.c_str(), &file_data, &file_size);
        if (result != 0) {
            return std::make_error_code(std::errc::io_error);
        }
        
        // Transfer ownership of the data to the vector
        data.assign(file_data, file_data + file_size);
        free(file_data);
        
        return {};
    }
    
private:
    EFI_SIMPLE_NETWORK* snp_ = nullptr;
    MacAddress mac_;
};

std::unique_ptr<NetworkInterface> NetworkInterface::create() {
    return std::make_unique<UefiNetworkInterface>();
}

// IPv4Address implementation
IPv4Address::IPv4Address(uint32_t addr) {
    octets_[0] = (addr >> 24) & 0xFF;
    octets_[1] = (addr >> 16) & 0xFF;
    octets_[2] = (addr >> 8) & 0xFF;
    octets_[3] = addr & 0xFF;
}

IPv4Address::IPv4Address(std::string_view str) {
    uint32_t a, b, c, d;
    if (sscanf(str.data(), "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        throw std::invalid_argument("Invalid IPv4 address format");
    }
    octets_ = {static_cast<uint8_t>(a), static_cast<uint8_t>(b),
               static_cast<uint8_t>(c), static_cast<uint8_t>(d)};
}

std::string IPv4Address::toString() const {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", 
             octets_[0], octets_[1], octets_[2], octets_[3]);
    return std::string(buf);
}

uint32_t IPv4Address::toUint32() const {
    return (octets_[0] << 24) | (octets_[1] << 16) | (octets_[2] << 8) | octets_[3];
}

// MacAddress implementation
std::string MacAddress::toString() const {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             octets_[0], octets_[1], octets_[2],
             octets_[3], octets_[4], octets_[5]);
    return std::string(buf);
}

// PXEClient implementation
PXEClient::PXEClient(std::unique_ptr<NetworkInterface> iface)
    : iface_(std::move(iface)) {}

PXEClient::~PXEClient() {
    if (iface_) {
        iface_->shutdown();
    }
}

std::error_code PXEClient::discoverNetwork() {
    if (!iface_) {
        return std::make_error_code(std::errc::not_connected);
    }
    
    auto ec = iface_->initialize();
    if (ec) return ec;
    
    ec = iface_->pxeDiscover(config_);
    if (!ec) {
        initialized_ = true;
    }
    
    return ec;
}

std::error_code PXEClient::downloadFile(const std::string& path, std::vector<uint8_t>& data) {
    if (!initialized_) {
        auto ec = discoverNetwork();
        if (ec) return ec;
    }
    
    return iface_->tftpDownload(config_.tftp_server, path, data);
}

std::error_code PXEClient::bootKernel(const std::string& kernel_path, 
                                    const std::string& initrd_path,
                                    const std::string& cmdline) {
    std::vector<uint8_t> kernel_data;
    std::vector<uint8_t> initrd_data;
    
    // Download kernel
    auto ec = downloadFile(kernel_path, kernel_data);
    if (ec) return ec;
    
    // Download initrd if specified
    if (!initrd_path.empty()) {
        ec = downloadFile(initrd_path, initrd_data);
        if (ec) return ec;
    }
    
    // Boot the kernel
    int result = pxe_boot_kernel(kernel_path.c_str(), 
                                initrd_path.empty() ? nullptr : initrd_path.c_str(),
                                cmdline.c_str());
    
    if (result != 0) {
        return std::make_error_code(std::errc::io_error);
    }
    
    return {};
}

} // namespace BloodHorn::Net
