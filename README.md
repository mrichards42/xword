
# XWord - A cross platform crossword solving program #

XWord is a cross-platform crossword puzzle program designed to be highly
configurable.  It reads multiple puzzle file formats.

## See it in action ##

In 2013 [Alex Boisvert](http://www.alexboisvert.com/) and
[Tyler Hinman](http://www.tylerhinman.com/) made a series of videos called
["Crossword Race"](https://www.youtube.com/watch?v=3kz5z7mobBM&list=PLkuKuYfrweip0hxFZr4gQ7Fk_eLrfbo2Z)
where they race to solve puzzles using XWord, with Tyler having different
handicaps each time.  Some of the handicaps showcase features of XWord and the
puzzle formats it supports, including [non-standard word directions](https://www.youtube.com/watch?v=3kz5z7mobBM&list=PLkuKuYfrweip0hxFZr4gQ7Fk_eLrfbo2Z)
and [diagramless solving](https://www.youtube.com/watch?v=7P1EeDniH5c&index=2&list=PLkuKuYfrweip0hxFZr4gQ7Fk_eLrfbo2Z).

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
(statically linked). wxaui-tweaks.patch slightly alters aui/framemanager.
wxWidgets-osx-private-build-fix.patch is an upstream patch to wxWidgets 3.1.3
which fixes Mac builds. If you would like to use these patches, apply them to
the wxWidgets directory.

XWord uses [premake](http://industriousone.com/premake/download) to generate
project files.  Version 4 has been used successfully, but the 5 beta may also work.

To build project files:

    $ premake [target]

    # Examples:
    $ premake4 vs2008
    $ premake4 xcode4

On windows, the %WXWIN% environmental variable will be used to find the wxWidgets
directory.  On other platforms, wx-config is used.  You can override either
%WXIN% or the default wx-config (see options below).

XWord-specific premake options:

    --disable-lua
    --wx-prefix="wxWidgets directory"
    --wx-config="wx-config path" # mac/linux

    # List all options
    $ premake4 --help

## Dependencies ##

Versions listed are the most recent that I have build XWord against.
I'm sure the simpler libraries could be updated.

Windows builds include dlls for everything except wxWidgets.

* wxWidgets (3.0; 3.1.2 is required for Mac OS 10.12+)
* lua (5.1, but actually using LuaJIT 2.0.3)
* expat
* curl (7.24.0)
* yajl (2.0.2 -- source included)
* zlib (1.2.5)
* libyaml (0.2.2 -- source included)

A debug build of wxWidgets is needed for debug builds of XWord, and likewise for release builds. The
following configure options should work as a baseline for release builds:

    $ ./configure --disable-shared --enable-compat28

For debug builds, add --enable-debug. For Mac OS builds, add --enable-universal-binary=i386
--with-macosx-version-min=10.6. You may also want to add -fvisibility-inlines-hidden to CFLAGS,
CXXFLAGS, CPPFLAGS, OBJCFLAGS, and OBJCXXFLAGS, as this is the default option used by the Mac
toolchain, and this option should be used consistently across an app and all of its dependencies.

### Lua libraries ###

Source included in lua directory

* luacurl (rename library to c-luacurl -- there is an additional lua module for this library)
* luatask (rename library to c-task -- there is an additional lua module for this library)
* lyaml (rename library to c-yaml -- there is an additional lua module for this library)
* luayajl
* lxp
* lfs
* wxLua (2.8.12.3 -- with bindings built against wxWidgets 3.0)
