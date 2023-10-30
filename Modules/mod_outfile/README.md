mod_outfile
====

Allow to control external devices exposed as files by Unix kernel.

### Accepted global directives
none

## Section OutFile

* **Topic=** Topic to listen to
* **File=** target file that where received data are written to
* **Func=** user validation function (see below, **mod_Lua** needed))
* **Disabled** start this section disabled

## User function
### Arguments

* **sectionid** Section identifier
* **payload** Received payload

### Returned value

* **1** *boolean* :
  * if **true** data is to be written
  * if **false** the file remain untouched

## Error condition

An error condition is associated to each section, individually. It is raised if a technical issue prevents to write data and is cleared as soon as an attempt succeed.

Error condition is exposed to Lua by **OutFile:inError()** methods.
