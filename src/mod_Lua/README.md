# mod_Lua
Add Lua's plugins to Marcel, allowing user functions

### Accepted global directives
* **UserFuncScript=** script to be executed to define user functions.

⚠️Notez-Bien⚠️ : 
  * only the last **UserFuncScript** is considered. If you want to load 
several scripts, provided **LoadAllScripts.lua** will load all script found in the same directory
  * callback functions are launched in the same Lua's state. Consequently, all objects are available 
to others but functions **have** to be as fast as possible (as they will block other one until completion)
