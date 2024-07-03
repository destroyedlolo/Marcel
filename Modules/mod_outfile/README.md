mod_outfile
====

Allow the Unix kernel to control external devices exposed as files by the Unix kernel.

### Accepted global directives
none

## Section OutFile

* **Topic=** Topic to listen to
* **File=** target file where received data is written to
* **Func=** user validation function (see below; **mod_Lua** needed)
* **Disabled** start this section disabled
* **DoNotSimulate** disables this section when running in simulation mode.

## User function
### Arguments

* **sectionid** Section identifier
* **payload** Received payload

### Returned value

* **1** *boolean* :
  * if **true** data is to be written
  * if **false** the file remain untouched

## Error condition

An error condition is associated with each section, individually. It is raised if a technical issue prevents to write data and is cleared as soon as an attempt succeed.

The error condition is exposed to Lua by the **OutFile:inError()** methods.
