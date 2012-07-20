Packages and Updates
====================

### Packages ###

The functionality of XWord can be extended through packages.
Examples of packages include:

- Puzzle downloader
- OneAcross, Wikipedia, and Google search
- Enhanced grid navigation, e.g.
    - Find clues that reference the current clue
    - Find the next blank square

To add, remove, or update extension packages, select `Package Manager` from
the [`Tools`](window.html#tools_menu) menu.  When XWord is run for the first
time, the Package Manager will pop up and inform the user of available packages.


### Updates ###

When XWord is started, it automatically checks for updates to the main program
and for installed packages.  This behavior cannot currently be changed, but if
an update is ignored, XWord will no longer ask the user to install the update
(though you will still be informed of future updates).


### For Developers ###

Packages are written in the [Lua](http://www.lua.org) programming language.
While there isn't really any documentation on the package system, the
XWord Debug package can be used to poke around.
[wxLua](http://wxlua.sourceforge.net/) is included for GUI development,
and bindings to the puzzle library written for XWord are included.  Browse the
`puz` and `xword` tables for available functions and methods.  The lua
libraries that are available for development are found in the scripts\libs
directory.