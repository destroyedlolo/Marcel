mod_inotify
====

Notification for file system changes like file creation, modification or removal.

### Accepted global directives

* **LookForChangesGrouped** If set, all LookForChanges are grouped, meaning **Marcel** will look up the first one in its configuration and stop L4C list as soon as it found a section  with a different type.<br>
It's for optimization purposes.

To do this, just **group all the LookForChanges section files at the same level** 
(for example, `55_???` while the classic sections are defined at the level `50_???`)

## Section LookForChanges

Send a notification when directory content changes. Also, applicable to a single file.

### Accepted directives

* **Topic=** Topic to publish to
* **On=** Directory (or file) to watch
* **Dir=** alias for **On=**
* **For=** *Actions* we are looking for
  * **create** File creation
  * **remove** File removal
  * **modify** A file has been modified
* **Func=** Acceptation function (**mod_Lua** needed)
* **Retained** submitted as retained message
* **Disabled** This section is disabled when Marcel's starting
*  **DoNotSimulate** disables this section when running in simulation mode.

### Published content

For each file modification, a notification is sent 
`file:action,action...`<br>
With `action` as defined by [Linux' INotify's](https://man7.org/linux/man-pages/man7/inotify.7.html) : 
`ACCESS`, `ATTRIB`, `CLOSE_NOWRITE`, ...

### Lua function arguments

1. Section ID
2. File name
3. comma-separated `Action` list

### Lua function return

* `true` the notification will be published
* `false` the notification will not be published

## Error condition

Unlike other modules, there is no error condition associated with a section handled by mod_inotify, but a global error state at the module itself.

It can be tested using the following code :
```
mod_inotify:inError()
```
