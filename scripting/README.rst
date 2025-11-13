Scripting Engine
===============

Overview
--------
The scripting engine provides a simple way to extend bootloader functionality
through custom scripts. It includes a basic lexer, parser, and API bindings.

Components
----------

Scripting Module
================
This module provides Lua scripting support for the BloodHorn Bootloader.

Core Components
---------------

lua.c/h
~~~~~~~
- Lua virtual machine integration
- Core Lua API bindings
- Script execution context
- Memory management

Script API (script_api.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~
- BloodHorn-specific Lua API bindings
- System functions exposure
- Bootloader integration points
- Security sandboxing

Script Environment (script_env.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Runtime environment management
- Script isolation
- Global state management
- Error handling

Script Loader (script_loader.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Script discovery and loading
- Dependency resolution
- Script lifecycle management
- Resource cleanup
- Control flow analysis

API Bindings (script_api.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- System functions
- Bootloader integration
- Memory operations
- I/O functions

Script Format
-------------
```
# Example script
print("Hello, BloodHorn!");

# Variable assignment
let boot_device = "hd0";

# Conditional execution
if (boot_device == "hd0") {
    boot("/boot/kernel");
} else {
    error("No boot device found");
}
```

API Reference
------------

Core Functions
~~~~~~~~~~~~~
- `print(string)` - Output text to console
- `error(string)` - Report an error
- `exit(code)` - Terminate script execution

System Functions
~~~~~~~~~~~~~~~
- `boot(kernel_path)` - Boot the specified kernel
- `read_file(path)` - Read file contents
- `write_file(path, data)` - Write data to file
- `execute(command)` - Execute a shell command

Memory Functions
~~~~~~~~~~~~~~~
- `alloc(size)` - Allocate memory
- `free(ptr)` - Free allocated memory
- `memcpy(dest, src, size)` - Copy memory

Usage
-----
```c
// Initialize scripting engine
script_engine_init();

// Load and execute a script
script_t *script = script_load("boot.script");
if (script) {
    script_execute(script);
    script_free(script);
}

// Clean up
script_engine_cleanup();
```

Error Handling
-------------
- Syntax errors are reported with line numbers
- Runtime errors include stack traces
- Memory usage is tracked and limited
- Script execution can be aborted

Limitations
-----------
- Single-threaded execution
- Limited standard library
- No dynamic loading of scripts
- Basic type system

See Also
--------
- :doc:`../boot/README`
- :doc:`../fs/README`
- :doc:`../recovery/README`
