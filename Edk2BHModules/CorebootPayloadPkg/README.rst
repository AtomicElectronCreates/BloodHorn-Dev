.. _corebootpayloadpkg:

Coreboot Payload Package
=======================

.. contents:: Table of Contents
   :depth: 3
   :local:

Overview
--------
The CorebootPayloadPkg provides the necessary infrastructure to run EDK2-based
firmware as a Coreboot payload. This package handles the integration between
Coreboot's native environment and the UEFI/PI environment expected by EDK2.

Features
--------
- **Automatic Detection**: Automatically detects when running as a Coreboot payload
- **Memory Management**: Translates Coreboot memory maps to UEFI memory maps
- **Hardware Initialization**: Leverages Coreboot's hardware initialization
- **Table Parsing**: Parses Coreboot tables and makes them available to UEFI
- **Hybrid Mode**: Maintains UEFI compatibility while using Coreboot services

Components
----------
- **CorebootPayloadLib**: Core library for Coreboot payload support
- **CorebootPayloadDxe**: DXE driver for Coreboot payload services
- **CorebootTables**: Support for parsing and interpreting Coreboot tables

Build Instructions
-----------------
To build with Coreboot payload support, include the following in your platform's DSC file:

.. code-block:: dsc

  [Components]
    CorebootPayloadPkg/CorebootPayloadDxe/CorebootPayloadDxe.inf

  [LibraryClasses]
    CorebootPayloadLib|CorebootPayloadPkg/Library/CorebootPayloadLib/CorebootPayloadLib.inf

Build with the following flag to enable Coreboot support:

.. code-block:: bash

  build -p YourPlatform.dsc -a X64 -b RELEASE -t GCC5 -D COREBOOT_PAYLOAD_ENABLE=1

API Reference
------------

CorebootPayloadLib
~~~~~~~~~~~~~~~~~~

.. c:function:: EFI_STATUS CorebootPayloadInitialize (VOID)

  Initialize the Coreboot payload environment.

.. c:function:: VOID *CorebootFindTable (UINT32 Tag, UINT32 *Size)

  Find a Coreboot table by tag.

  :param Tag: The table tag to find
  :param Size: Optional pointer to return the table size
  :return: Pointer to the table or NULL if not found

Configuration
------------
The following PCDs are available for configuration:

- ``gCorebootPayloadPkgTokenSpaceGuid.PcdCorebootTableAddress``: 
  Manual override for Coreboot table address (0 for auto-detect)

Dependencies
-----------
- MdePkg
- MdeModulePkg
- UefiDriverEntryPoint

Testing
------
To verify Coreboot payload functionality:

1. Build with Coreboot support enabled
2. Load as a payload in Coreboot
3. Check the debug log for Coreboot payload initialization messages
4. Verify memory maps and hardware information are correctly reported

Limitations
----------
- Currently supports X64 architecture only
- Limited ACPI/device tree support in Coreboot mode

License
-------
SPDX-License-Identifier: BSD-2-Clause-Patent

Copyright (c) 2025, BloodHorn Developers. All rights reserved.
