
# XWord - A cross platform crossword solving program #

XWord is a cross-platform crossword puzzle program designed to be highly
configurable.  It reads multiple puzzle file formats.

## Features ##

* Solving of all American style, cryptic, and diagramless crosswords.
* Solving rebus puzzles.
* Solving puzzles with a variety of tricks, e.g. diagonal clues, arbitrary
  numbering schemes, shaded squares.
* Support for reading number of puzzle formats:
    * Across Lite
    * XPF
    * jpz
    * ipuz (partial)
* Recovery of corrupted Across Lite .puz files.
* A timer.
* Checking and revealing letters.
* Scrambling and unscrambling puzzle solutions (for Across Lite files).
* Viewing puzzle notes.
* Zooming the grid in and out.
* Rearranging the window layout.
* Color, font, and solving behavior configuration.
* Printing

XWord is extensible via the lua scripting language.  A number of plugins are
available.  See Tools > Package Manager.


### Diagramless Puzzles ###

XWord allows the user to solve diagramless puzzles similarly to normal puzzles
with the exception that black squares are allowed to be selected. When
selected, black squares are outlined in the focused square color.
To make a white square black, type a period (".").  To make a black square
white, type a period or enter another letter.

XWord provides some guidance when solving diagramless puzzles: clue numbers
in the grid are automatically calculated when a square is made black or white.
Numbers in the grid that do not appear in the clue list are highlighted in red.
Naturally, this automatic calculation of grid numbers only works if the puzzle
follows the normal rules of numbering a grid.


# Building XWord #

XWord uses the [wxWidgets](http://www.wxwidgets.org) cross-platform toolkit
(statically linked).  On windows, it is easiest to set a `WXWIN` environmental
variable.  On other platforms, use wx-config.

The wxpatches directory contains slightly altered versions of aui/framemanager.
If you would like to use these patches, copy them to the wxWidgets directory
and overwrite the corresponding files.

XWord uses [premake](http://industriousone.com/premake/download) to generate
project files.  Version 4 has been used successfully, but the 5 beta may also work.

To see all options:

    $ cd xword
    $ premake4 --help

To build project files:

    $ premake [target]

    # Examples:
    $ premake4 vs2008
    $ premake4 xcode4

XWord-specific premake options:

    --disable-lua
    --wx-prefix="wxWidgets directory"

## Dependencies ##

Versions listed are the most recent that I have build XWord against.
I'm sure the simpler libraries could be updated.

Windows builds include dlls for everything except wxWidgets.

* wxWidgets (3.0)
* lua (5.1, but actually using LuaJIT 2.0.3)
* expat
* curl (7.24.0)
* yajl (2.0.2 -- source included)
* zlib (1.2.5)

### Lua libraries ###

Source included in lua directory

* luacurl (rename library to c-luacurl -- there is an additional lua module for this library)
* luatask (rename library to c-task -- there is an additional lua module for this library)
* luayajl
* lxp
* lfs
* wxLua (2.8.12.3 -- with bindings built against wxWidgets 3.0)


## Windows notes ##

The images and scripts directories need to be in the same directory as XWord.exe.
It is easiest to create symlinks so that changes are shared -- this requires
administrator privileges.

    cd xword
    mkdir bin\Debug
    mklink /d bin\Debug\images images
    mklink /d bin\Debug\scripts scripts
    mkdir bin\Release
    mklink /d bin\Release\images images
    mklink /d bin\Release\scripts scripts
