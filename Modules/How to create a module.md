# Module creation

| :relaxed:        | This documentation is highly technical and is intended for developers looking to create custom modules or those interested in Marcel’s internal architecture. |
--- | --- |

Starting with V8, Marcel transitioned to a robust, dynamically loaded module system. This architecture reduces the system's memory
footprint by loading only necessary code, streamlines development, and enhances overall security. This guide explains how to extend
Marcel’s capabilities by creating your own module, using **mod_dummy** as a reference. Please note that this document outlines the
module's skeleton; for a deep dive, please refer to the detailed comments within the source code.

## Structural concepts

### Versioning

The version is set at Marcel's level (if a module is touched, the global Marcel's version is bumped)We follow a MAJOR.MI.SB versioning scheme: `MAJOR.MISB`

- *MAJOR*: Indicates a structural change. For instance, V8 introduced dynamic loadable modules and a complete overhaul of configuration files. **Backward compatibility is not guaranteed for major releases**, though we strive to avoid "big bangs" whenever possible :smirk:
- *MI*nor : Indicates new features. **Backward compatibility is strictly enforced** between minor versions. 
- *S*u*B* version : Indicates bug fixes or minor adjustments that do not impact Marcel's core logic or configuration. These are primarily internal improvements.

### Shared objects

Technically, modules are **shared objects (.so)** loaded on demand via Marcel's `LoadModule=` directive. The only exposed entry
point is `InitModule()`, which initializes the module's internal structure; all further interactions are handled through **callbacks**.

The primary advantage of this architecture (*inspired by Amiga libraries*) is the **total decoupling** of Marcel’s core logic from its
modules. This avoids the tight linking typically found in standard Linux shared libraries.

### Development Guidelines

Please keep the following principles in mind when creating a new module:
- **No Inter-Module Sharing**: Modules must remain isolated. There should be no shared global variables or cross-module
function calls. The only exception is **mod_lua**.
- **Non-Blocking Design**: Avoid long-running or blocking processes to ensure the system remains responsive.
- **Resource Efficiency**: Be conservative with memory and CPU usage to maintain a lightweight footprint.
- **Documentation**: Every module must include its own `README.md` file within its directory.
- **Configuration**: Provide a well-documented configuration skeleton within a `/Config` subdirectory.

## Creating a module step by step

### Module name

Choose a meaningful module name and create, in `Modules`, its own `mod_???` directory.

In our example, the module name is **dummy** so we need to create
```
mkdir Modules/mod_dummy
```

### Update Makefile creation script (remake.sh)

**remake.sh** is a shell script where you can configure which modules have to be built and updates Makefiles accordingly. Several sections need to be updated as bellow.

#### Configuration area

The **Configuration area** is where you can tell `remake.sh` which modules to build.

```
# Example plugin
# This one is strictly NO-USE. Its only purpose is to demonstrate how to build a plugin
BUILD_DUMMY=1
```

#### Development related

This part contains general build options like compiler flags, where to find external components ... As a matter of fact, few modules
have needs here, and it's not the case of our dummy example.

#### Rebuild Makefiles

Each `mod_` directory contains its own Makefile. This section only adds them into the global one. That is, a `make` from the project's home directory will build all modules as well.

```
if [ ${BUILD_DUMMY+x} ]; then
	echo -e '\t$(MAKE) -C Modules/mod_dummy' >> Makefile
fi
```

#### Rebuild modules' own Makefile

This section will re-generate our module's Makefile using my **LFMakeMaker** (this tool automatically create a makefile as per dependancies it found in your source code).

```
if [ ${BUILD_DUMMY+x} ]; then
	cd Modules/mod_dummy
	LFMakeMaker -v +f=Makefile --opts="$CFLAGS $LUA $DEBUG $MCHECK" *.c -so=../../mod_dummy.so > Makefile
	cd ../..
fi
```

### Module header (.h)

The C header defines all objects and interfaces belonging to a module.

#### includes

Most of shared include files are stored in `Marcel` directory. Our dummy module needs
- `#include "../Marcel/Module.h"` modules' handler
- `#include "../Marcel/Section.h"` we want to define custom sections

#### module structure

```
struct module_dummy {
	struct Module module;
```
Each module definition **must** start with a *module handler structure*, which contains important information like its name and callbacks.

In addition, it may contain some custom fields used to store module's parameters, internal status, etc ... Our dummy module will create 2 fields :

```
	int test;	/* variable containing interesting stuffs for the module */
	bool flag;
};
```

#### section structure

Our dummy module can handle 2 sections, impersonated by dedicated structures.

```
struct section_dummy {
	struct Section section;
```

Again, they must start with a section handler to store its name, internal technical stuffs and configuration fields usable among all sections (as `disabled`).

It may contain as well some custom fields.
```
		/* Variables dedicated to this structure */
	int dummy;
}
```
### Module main code (mod_dummy.c)

#### Instantiate module own structure

```
static struct module_dummy mod_dummy;
```

#### Enumerate sections' identifiers

Each section must have a unique identifier per module. Only up to 256 sections can be defined for a single module, which is farther than enough.

```
enum {
	ST_DUMMY = 0,
	ST_ECHO
};
```

#### Initialization function - InitModule()

**InitModule()** MUST exist in each module and aims to initialize its custom fields, callbacks and then register the module for its activation inside Marcel.

```
void InitModule( void ){
```

##### Module's initialization

Name our module and set its structure fields to safe value.

```
	initModule((struct Module *)&mod_dummy, "mod_dummy");
```

Initialize used callbacks

```
	mod_dummy.module.readconf = readconf;
	mod_dummy.module.acceptSDirective = mt_acceptSDirective;
	mod_dummy.module.getSlaveFunction = mt_getSlaveFunction;
```

Register our module

```
	registerModule( (struct Module *)&mod_dummy );
```

Optionally, set some custom fields.

```
	mod_dummy.test = 0;
	mod_dummy.flag = false;
```

```
}
```

-----
| :construction_worker: To be continued :construction_worker: |
----
