Network Stack
============

Overview
--------
The network module provides networking capabilities to the BloodHorn bootloader, enabling
network booting (PXE) and remote file access. It implements essential networking protocols
and provides a clean interface for network operations.

Features
--------
- PXE (Preboot eXecution Environment) boot support
- DHCP client for automatic IP configuration
- ARP for address resolution
- TFTP client for file transfer
- UEFI network protocol wrappers

Core Components
---------------

PXE Boot (pxe.c/h, pxe_rm.asm)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Handles PXE boot protocol
- Manages UNDI network interface
- Provides real-mode assembly helpers for PXE calls
- Implements PXE API translation layer

DHCP Client (dhcp.c/h)
~~~~~~~~~~~~~~~~~~~~~~
- Implements DHCP protocol (RFC 2131)
- Handles IP address assignment
- Supports DHCP options and vendor extensions
- Manages lease time and renewal

ARP (arp.c/h)
~~~~~~~~~~~~~
- Implements Address Resolution Protocol (RFC 826)
- Maintains ARP cache
- Handles ARP requests and replies
- Provides MAC address resolution

TFTP Client (tftp.c/h)
~~~~~~~~~~~~~~~~~~~~~~
- Implements Trivial File Transfer Protocol (RFC 1350)
- Supports block number rollover
- Handles error conditions and retransmissions
- Provides file download functionality

UEFI Network (uefi_network.cpp, network.hpp)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Wraps UEFI network protocols
- Provides C++ interface for network operations
- Manages network interface state
- Handles protocol binding and events

Network Utilities (net_utils.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Common network utilities
- Endianness conversion
- Network address manipulation
- Debugging helpers

Dependencies
------------
- UEFI Network Protocol Stack
- UEFI PXE Base Code Protocol
- UEFI Simple Network Protocol
- Memory allocation services
- Timer services

Building
--------
The network module is built as part of the main BloodHorn build process. It requires
UEFI network stack support in the firmware.

Usage Example
-------------
```c
#include "net/network.hpp"
#include "net/dhcp.h"

// Initialize network stack
NetworkInterface net;
if (net.initialize() != EFI_SUCCESS) {
    // Handle error
}

// Perform DHCP
struct dhcp_packet *dhcp = dhcp_discover();
if (!dhcp) {
    // Handle error
}

// Download file via TFTP
struct tftp_context ctx;
if (tftp_get(&ctx, "192.168.1.1", "boot/kernel.elf", buffer, sizeof(buffer)) < 0) {
    // Handle error
}

// Clean up
net.shutdown();
```

Security Considerations
----------------------
- Network operations run with boot services privileges
- No encryption in TFTP (use secure boot for integrity)
- Validate all network input
- Implement rate limiting for network operations

Documentation
-------------
See header files for detailed API documentation:
- ``network.hpp``: Main network interface
- ``pxe.h``: PXE boot protocol
- ``dhcp.h``: DHCP client
- ``arp.h``: ARP protocol
- ``tftp.h``: TFTP client

For protocol specifications, refer to the relevant RFCs:
- PXE: Preboot Execution Environment (PXE) Specification v2.1
- DHCP: RFC 2131
- ARP: RFC 826
- TFTP: RFC 1350
