EDK2 Modules for BloodHorn
=========================

.. image:: https://img.shields.io/badge/EDK2-Modified%20for%20BloodHorn-blue
    :target: https://github.com/tianocore/edk2
    :alt: EDK2 Modified for BloodHorn

Overview
--------
This directory contains a modified version of the `EDK2 (EFI Development Kit II) <https://github.com/tianocore/edk2>`_ framework, customized for the BloodHorn bootloader. These modifications provide essential UEFI services and hardware abstraction while maintaining compatibility with the UEFI specification.

Key Components
--------------

Core Packages
~~~~~~~~~~~~~
- **MdePkg**: Core UEFI package with base definitions and libraries
- **MdeModulePkg**: Standard UEFI module implementations
- **CryptoPkg**: Cryptographic services and algorithms

BloodHorn-Specific Additions
~~~~~~~~~~~~~~~~~~~~~~~~~~~
- **CorebootPayloadPkg**: Support for Coreboot integration
- **Custom Build Tools**: Modified build system for BloodHorn's requirements

Building
--------
To build BloodHorn with these EDK2 modules:

1. Set up the build environment:

   .. code-block:: bash

       # Initialize the EDK2 environment
       . edksetup.sh

2. Build BloodHorn:

   .. code-block:: bash

       # For x86_64 with GCC
       build -p BloodHorn.dsc -a X64 -b RELEASE -t GCC5

       # For ARM64 with GCC
       build -p BloodHorn.dsc -a AARCH64 -b RELEASE -t GCC5

Directory Structure
------------------
::

    Edk2BHModules/
    ├── BaseTools/          # Modified build tools
    ├── MdePkg/            # Core UEFI package
    ├── MdeModulePkg/      # UEFI module implementations
    ├── CryptoPkg/         # Cryptographic services
    ├── EmbeddedPkg/       # Embedded platform support
    ├── UefiCpuPkg/        # CPU-specific UEFI drivers
    └── CorebootPayloadPkg/ # Coreboot integration support

License
-------
This modified EDK2 code is distributed under the `BSD-2-Clause-Patent License <https://opensource.org/licenses/BSDplusPatent>`_, the same as the original EDK2 project.

.. note::
    This is a modified version of EDK2. For the original source, visit the `official EDK2 repository <https://github.com/tianocore/edk2>`.
