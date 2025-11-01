EDK2 Modules for BloodHorn
=========================

.. image:: https://img.shields.io/badge/EDK2-Modified%20for%20BloodHorn-blue
    :target: https://github.com/tianocore/edk2
    :alt: EDK2 Modified for BloodHorn

Overview
--------
This directory contains a modified version of the `EDK2 (EFI Development Kit II) <https://github.com/tianocore/edk2>`_ framework, specifically tailored for the BloodHorn bootloader. The original EDK2 code has been customized to better suit the needs of BloodHorn while maintaining compatibility with the UEFI specification.

Modifications from Upstream EDK2
--------------------------------

Core Changes
~~~~~~~~~~~~
- **Code Optimization**: Critical paths have been optimized for faster boot times
- **Memory Management**: Enhanced memory allocation and management
- **Security Enhancements**: Additional security measures and hardening
- **Coreboot Integration**: Improved support for Coreboot as a payload
- **BloodHorn-Specific Drivers**: Custom drivers for BloodHorn's hardware support

Structure
---------
::

    Edk2BHModules/
    ├── BaseTools/          # Modified build tools
    ├── MdePkg/            # Core UEFI package
    ├── MdeModulePkg/      # UEFI module implementations
    ├── CryptoPkg/         # Cryptographic services
    └── SecurityPkg/       # Security-related modules

Building
--------
To build BloodHorn with these modified EDK2 modules:

.. code-block:: bash

    # Set up the build environment
    source edksetup.sh

    # Build the target platform
    build -p BloodHornPkg/BloodHorn.dsc -a X64 -b RELEASE -t GCC5

License
-------
This modified EDK2 code is distributed under the `BSD-2-Clause-Patent License <https://opensource.org/licenses/BSDplusPatent>`_, the same as the original EDK2 project.

.. note::
    This is a modified version of EDK2. For the original, unmodified source, please visit the `official EDK2 repository <https://github.com/tianocore/edk2>`.
