Plugin System
============

Overview
--------
The plugin system allows extending BloodHorn's functionality through loadable modules.
Plugins can add new filesystems, network protocols, security features, and more without
modifying the core bootloader code.

Features
--------
- Dynamic loading of plugins
- Version checking
- Dependency resolution
- Event-based architecture
- Resource management

Plugin Types
------------

Filesystem Plugins
~~~~~~~~~~~~~~~~~~
- Add support for new filesystems
- Implement custom storage backends
- Virtual filesystems

Network Plugins
~~~~~~~~~~~~~~
- Additional network protocols
- VPN support
- Network boot methods

Security Plugins
~~~~~~~~~~~~~~~
- Authentication methods
- Encryption schemes
- Secure boot extensions

UI Plugins
~~~~~~~~~~
- Custom boot menus
- Themes and skins
- Input methods

Creating a Plugin
-----------------
1. Create a new directory in the plugins directory
2. Add a `plugin.ini` file with metadata
3. Implement the required plugin interface
4. Build as a shared library (.so, .dll, or .efi)

Example `plugin.ini`:
```ini
[Plugin]
Name = Example Plugin
Type = filesystem
Version = 1.0.0
Author = Your Name
Description = Example filesystem plugin
Dependencies = fs_core
API_Version = 1
```

Plugin API
----------
```c
#include <plugins/plugin.h>

// Plugin entry point
int plugin_init(struct plugin_ctx *ctx) {
    // Register callbacks, add commands, etc.
    return PLUGIN_SUCCESS;
}

// Plugin cleanup
void plugin_cleanup(void) {
    // Release resources
}
```

Building Plugins
---------------
```makefile
# Example Makefile for a plugin
PLUGIN_NAME = my_plugin
PLUGIN_TYPE = filesystem

SRCS = my_plugin.c
OBJS = $(SRCS:.c=.o)

CFLAGS += -fPIC -I$(BLOODHORN_SRC)/include
LDFLAGS += -shared

all: $(PLUGIN_NAME).so

$(PLUGIN_NAME).so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
```

Loading Plugins
--------------
Plugins can be loaded in several ways:

1. From the bootloader configuration:
   ```
   plugin_load /path/to/plugin.so
   ```

2. At runtime (if supported):
   ```
   load_plugin /path/to/plugin.so
   ```

3. Automatically from a plugins directory

Security Considerations
----------------------
- Plugins run with the same privileges as the bootloader
- Validate all plugin metadata
- Verify plugin signatures if secure boot is enabled
- Isolate plugin memory when possible

Debugging
---------
Enable plugin debug output:
```c
setenv PLUGIN_DEBUG 1
```

See Also
--------
- :doc:`../security/README`
- :doc:`../fs/README`
- :doc:`../net/README`
