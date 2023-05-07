# Module creation

| :relaxed:        | This documentation is very technical and targets people wanting to create their own module or ones interested by Marcel's internal way of working |
--- | --- |

As of V8, Marcel moved to a strong dynamically loaded modules' architecture. 
This architecture reduces the system's footprint by avoiding loading unused pieces of code, makes development easier and,
all in all, increase modules' security.

This little document explains how to create your own module to improve Marcel's capabilities, taking **mod_dummy** as example.


## Structural concepts

### Versioning

The version is set at Marcel's level (if a module is touched, the global Marcel's version is bumped) using the following scheme : `MAJOR.MISB`

- *MAJOR* : if bumped, a structural change happened to Marcel. As an example, V8 introduced the concept of loadable module (versus hard-coded ones) and configuration files dramatically changed.
Upwards compatibility is not guaranteed, even if the best is done to avoid big bangs as of V8 :smirk:.
- *MI*nor : feature added. Upwards compatibility **is enforced** between minors versions. 
- *S*u*B* version : bug fix, small changes not impacting fundamental Marcel's way of working or configuration ... mostly internal changes.

### Shared objects

Modules technically are shared object (.so) that will be loaded on demand using Marcel's `LoadModule=` directive. The only exposed entry point is `InitModule()` which initialize module's internal structure : other exchanges are done using **callbacks**.

The added value of this kind of architecture (*inspired by the way Amiga's libraries*) is to avoid tight links between Marcel's own code and modules, as it would be the case with classical Linux shared libraries.

### Golden rules

Following guidelines need to be kept in mind when creating a module :
- no inter-module sharing : there is no need to share anything between modules (no global variable, no cross module functions, ...). The only exception is **mod_lua** and all exchanges is done using callbacks as well.
- avoid blocking long processing
- be resource conservative
- provide a module level `README.md` to document the module
- in a subdirectory `/Config`, provide comprehensible and documented configuration skeleton.

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

This part contains general build options like compiler flags, where to find external components ... As a matter of fact, few modules have needs here, and it's not the case of our dummy example.

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
