# Module creation

| :relaxed:        | This documentation is very technical and targets people wanting to create their own module or ones interested by Marcel's internal way of working |
--- | --- |

As of V8, Marcel moved to a strong dynamically loaded modules' architecture. 
This architecture reduces the system's footprint by avoiding loading unused pieces of code, makes development easier and,
all in all, increase modules' security.

This little document explains how to create your own module to improve Marcel's capabilities, taking **mod_dummy** as example.


## Structural concepts

## Versioning

The version is set at Marcel's level (if a module is touched, the global Marcel's version is bumped) using the following scheme : `MAJOR.MISB`

- *MAJOR* : if bumped, a structural change happened to Marcel. As an example, V8 introduced the concept of loadable module (versus hard-coded ones) and configuration files dramatically changed.
Upwards compatibility is not guaranteed, even if the best is done to avoid big bangs as of V8 :smirk:.
- *MI*nor : feature added. Upwards compatibility **is enforced** between minors versions. 
- *S*u*B* version : bug fix, small changes not impacting fundamental Marcel's way of working or configuration ... mostly internal changes.

## objects

Modules technically are .so shared object that will be loaded on demand using Marcel's `LoadModule=` directive. It's look only for `InitModule()` entry point to initialize module's internal structure : other exchanges are done using **callbacks**.

The added value of this kind of architecture (*inspired by the way Amiga's libraries*) is to avoid tight links between Marcel's own code and modules, as it would be the case with classical Linux shared libraries.
