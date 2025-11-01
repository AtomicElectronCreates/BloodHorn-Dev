Recovery Shell
=============

Overview
--------
The recovery shell provides a minimal command-line interface for system recovery and debugging.
It's designed to be lightweight and functional even in limited environments.

Components
----------

Shell Core (shell.c/h)
~~~~~~~~~~~~~~~~~~~~~
- Command line interface
- Input handling
- Command execution
- Basic line editing

Shell Commands (shell_cmds.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Built-in commands
- Command registration
- Help system
- Command history

Filesystem Commands (shell_fs.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- File operations
- Directory listing
- Filesystem navigation
- File content viewing

Network Commands (shell_net.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Network configuration
- Basic network testing
- Remote file access
- Network diagnostics

Command History (shell_history.c/h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Command recall
- History navigation
- Persistent history storage
- History search

Usage
-----
```c
// Initialize the recovery shell
shell_init();

// Process a command line
shell_execute("ls /boot");

// Run the interactive shell
shell_loop();
```

Built-in Commands
----------------
- `help` - Show available commands
- `ls` - List directory contents
- `cd` - Change directory
- `cat` - Display file contents
- `ifconfig` - Network interface configuration
- `ping` - Network connectivity test
- `history` - Command history

Dependencies
------------
- Basic I/O functions
- Memory allocation
- String manipulation
- Platform-specific system calls

Building
--------
The recovery shell is built as part of the main BloodHorn build process.
It's automatically included when recovery features are enabled.

Limitations
-----------
- Limited line editing capabilities
- Basic error handling
- Minimal built-in commands
- No command completion

See Also
--------
- :doc:`../fs/README`
- :doc:`../net/README`
- :doc:`../boot/README`
