/*
 * network.hpp
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

class IPv4Address {
public:
    constexpr IPv4Address() : octets_{0, 0, 0, 0} {}
    constexpr explicit IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
        : octets_{a, b, c, d} {}
    
    explicit IPv4Address(uint32_t addr);
    explicit IPv4Address(std::string_view str);
    
    // Copy/move constructors and assignment operators
    IPv4Address(const IPv4Address&) = default;
    IPv4Address& operator=(const IPv4Address&) = default;
    IPv4Address(IPv4Address&&) = default;
    IPv4Address& operator=(IPv4Address&&) = default;
    
    // Comparison operators
    bool operator==(const IPv4Address&) const = default;
    
    // Conversion
    std::string toString() const;
    uint32_t toUint32() const;
    
    // Accessors
    const std::array<uint8_t, 4>& octets() const { return octets_; }
    
    // Common addresses
    static IPv4Address any() { return IPv4Address(0, 0, 0, 0); }
    static IPv4Address loopback() { return IPv4Address(127, 0, 0, 1); }
    static IPv4Address broadcast() { return IPv4Address(255, 255, 255, 255); }
    
private:
    std::array<uint8_t, 4> octets_;
};

/**
 * @brief Represents a MAC address
 */
class MacAddress {
public:
    static constexpr size_t LENGTH = 6;
    
    constexpr MacAddress() : octets_{0} {}
    constexpr explicit MacAddress(const uint8_t bytes[LENGTH]) {
        for (size_t i = 0; i < LENGTH; ++i) {
            octets_[i] = bytes[i];
        }
    }
    
    // Copy/move constructors and assignment operators
    MacAddress(const MacAddress&) = default;
    MacAddress& operator=(const MacAddress&) = default;
    MacAddress(MacAddress&&) = default;
    MacAddress& operator=(MacAddress&&) = default;
    
    // Comparison operators
    bool operator==(const MacAddress&) const = default;
    
    // Accessors
    const std::array<uint8_t, LENGTH>& octets() const { return octets_; }
    std::string toString() const;
    
    // Special addresses
    static MacAddress broadcast() { return MacAddress({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}); }
    static MacAddress zero() { return MacAddress({0, 0, 0, 0, 0, 0}); }
    
private:
    std::array<uint8_t, LENGTH> octets_;
};

/**
 * @brief Network interface configuration
 */
struct NetworkConfig {
    IPv4Address ip_address;
    IPv4Address netmask;
    IPv4Address gateway;
    IPv4Address dns_server;
    MacAddress mac_address;
    
    // PXE specific
    std::string tftp_server;
    std::string boot_file;
    std::string domain_name;
};

/**
 * @brief Network interface class
 */
class NetworkInterface {
public:
    virtual ~NetworkInterface() = default;
    
    // Network operations
    virtual std::error_code initialize() = 0;
    virtual std::error_code shutdown() = 0;
    
    // ARP operations
    virtual std::error_code resolveIpToMac(const IPv4Address& ip, MacAddress& mac) = 0;
    
    // PXE operations
    virtual std::error_code pxeDiscover(NetworkConfig& config) = 0;
    virtual std::error_code tftpDownload(const std::string& server, const std::string& remote_path,
                                       std::vector<uint8_t>& data) = 0;
    
    // Create appropriate network interface based on system
    static std::unique_ptr<NetworkInterface> create();
};

/**
 * @brief PXE boot client
 */
class PXEClient {
public:
    explicit PXEClient(std::unique_ptr<NetworkInterface> iface);
    ~PXEClient();
    
    // Network boot operations
    std::error_code discoverNetwork();
    std::error_code downloadFile(const std::string& path, std::vector<uint8_t>& data);
    std::error_code bootKernel(const std::string& kernel_path, 
                              const std::string& initrd_path = "",
                              const std::string& cmdline = "");
    
    // Getters
    const NetworkConfig& config() const { return config_; }
    
private:
    std::unique_ptr<NetworkInterface> iface_;
    NetworkConfig config_;
    bool initialized_ = false;
};

// Utility functions
std::string formatMacAddress(const uint8_t* mac);
std::string formatIpAddress(const uint8_t* ip);

} // namespace BloodHorn::Net
