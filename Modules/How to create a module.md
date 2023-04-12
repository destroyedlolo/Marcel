# Module creation

As of V8, Marcel moved to a strong dynamically loaded modules' architecture. 
This architecture reduces the system's footprint by avoiding loading unused pieces of code, makes development easier and,
all in all, increase modules' security.

This little document explains how to create your own module to improve Marcel's capabilities.


## Structural concepts

## Versioning

The version is set at Marcel's level (if a module is touched, the global Marcel's version is bumped) using the following scheme : `MAJOR.MISB`

- *MAJOR* : if bumped, a structural change happened to Marcel. As an example, V8 introduced the concept of loadable module (versus hard-coded ones) and configuration files dramatically changed.
Upwards compatibility is not guaranteed, even if the best is done to avoid big bangs as of V8 :smirk:.
- *MI*nor : feature added. Upwards compatibility **is enforced** between minors versions. 
- *S*u*B* version : bug fix, small changes not impacting fundamental Marcel's way of working or configuration ... mostly internal changes.
