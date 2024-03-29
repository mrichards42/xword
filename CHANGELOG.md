Version 0.8.0 (11/29/2022)
==========================
Puzzle Features
---------------
* Add support for ipuz files with custom cells aka words (#199)
* Add support for cryptic and diagramless ipuz files (#181)
* Add support for cell background colors in ipuz files (#187)
* Add support for coded crosswords (aka "codewords") (#124)
* Preliminary support for puz v2 with support for utf-8 (#155)
* Support alternate background color for black squares (#148)
* Allow asterisk (`*`) in the solution -- this must be entered as a rebus since
  pressing `*` adds a circle to a square (#195)
* Remove version check from ipuz reader -- allows loading ipuz v2 files (#173)

New Features
------------
* Option to change direction when clicking on the current square (436ab09)
* Downs only tool (3f18eeb)

Improvements
------------
* HTML is stripped from the window title (#118)
* More intuitive initial cursor positioning on non-standard grids (#136 #138 #141)
* Annotation squares use a white background by default (instead of black) (#151)
* When converting to puz format, all notes-like fields are saved to the puz
  "Notes" field (#145)
* Better support for HTML in metadata fields (#193)

Puzzle Sources
--------------
* New Yorker updated to be every weekday
* Added New Yorker Cryptic
* Added Vulture 10x10
* Fixed NY Mag
* Removed extraneous enumerations from LA Times

Bug Fixes
---------
* Downloader settings can be saved (8af9e02)
* Fix italics getting cut off (#159)
* Extra whitespace no longer breaks metadata display (#119 #149)
* Fix crash when saving rebus entry on Linux (#130)
* Fix save behavior on Windows for files with accents (#135)
* Improve handling of different capitalizations of "Across" and "Down" (#143)
* Fix several navigation edge cases (#147 #152 #153 #162 #194)
* Fix word length issue with barred borders (#175)
* Fix crash on latest MacOS version (note that this also changes the window
  style on macs) (#150)
* Fix crash entering a rebus in a square with no solution (#195)
* Fix namespace when saving JPZ files (#177)
* Fix issue with downloader puzzle popup and shared squares (1461066)
* Fix issue with annotation squares not drawing on printouts (#202)


----------------------------------------------------------------------


Version 0.7.0 (6/23/2020)
==========================
Puzzle Features
---------------
* Acrostic support (specially formatted .jpz)
* Rows Garden support (.rg and .rgz files)
* Support for barred square borders (.jpz and .ipz only)

New Features
------------
* Option to hide percent complete and completion alerts (e.g. for tournament
  training).
* Option to highlight theme clues.
* Fully supported Mac release.

Improvements
------------
* Rewrote downloader package and updated sources.
* Improved layout system: panes resize with the window, and clue lists are made
  to resize together.
* Rewrote preferences dialog.
* Show all notes-like fields in notes pane (e.g. description or instructions)
* Less harsh default colors.
* Improvements to hdpi screens.

Bug Fixes
---------
* JPZ roundtrips unknown metadata.
* Save formatted JPZ clues in a way that is compatible with crossword solver.
* Timer always pauses when the window is hidden.
* Show all supported file types in "open" dialog.


----------------------------------------------------------------------


Version 0.6.1 (8/12/2012)
==========================
New Features
------------
* More customizable printing, including an option to print on two-pages.


Bug Fixes
---------
* Cleaner update dialog that will not pop up after the user has ignored updates.
* DLLs for cURL SSL are included
* Fix occasional bug where raw HTML character referencs would show up in
  printouts.
* Circles are the same color and weight as grid lines.


----------------------------------------------------------------------


Version 0.6 (7/20/2012)
==========================
New Features
------------
* Copy and paste support.
* Rebus entry uses an expanding text ctrl so that the entire entry can be seen.
* Revised style preferences to be simpler and more comprehensive.
* Configurable metadata info panels.
* New menu item: Reset Timer.
* Puzzle status area shows percent complete for incomplete puzzles.
* Edit Mode for moving panels.
* More refined panel configuration.
* Lua scripts can insert panels into the preferences dialog.

Bug Fixes
---------
* Fixed regression where the last square in the grid was not being checked.
* If autosave fails for some reason, the user is alerted in the status bar
  instead of with a message box.
* Frame size and position are saved if frame is iconized when it is closed.
* Puz files with no solution are correctly not able to be checked.
* The grid redraws correctly when scrolled (if it is zoomed in)

Removed Features
----------------
* Character map removed (not all-inclusive, and somewhat buggy; use the system
  character map)


----------------------------------------------------------------------


Version 0.5.4 (11/16/2011)
==========================
New Features
------------
* New printing options: Current progress, blank grid, solution
* Letters that have been checked and found to be correct are indicated with
  a green circle in the top right corner.  These squares are not allowed to
  be overwritten.
* Panes resize proportionally when the window is resized.
* Help files
* New menu item: Erase Grid.
* New menu item: Erase Uncrossed Squares.
* New menu item / toolbar item: Rebus Entry (previously only accessible by
  pressing INSERT)

Bug Fixes
---------
* Fix regression where .puz files did not load sections (e.g. timer, circles).
* Diagramless puzzles: fixed crashes and/or error messages on loading.
* Diagramless puzzles: clicking on a clue no longer moves the cursor to the
  start of the word (revealing information about the grid).
* Odd numbering schemes are correctly displayed (XPF; see NYT 10/21/2011).

Removed Features
----------------
* Run Script menu item removed from the Tools menu (redundant: the package
  system should take care of all script needs).


----------------------------------------------------------------------


Version 0.5.3 (10/17/2011)
==========================
New Features
------------
* Option to save a history of recently opened files.
* Option to reopen the previous puzzle.

Bug Fixes
---------
* Fix major regression that prevents saving .puz files.
* Puzzles with background images can't be saved in XPF format.
* Puzzle loading errors do not include line information:
  e.g. "../import.init.lua line xxx: error message"


----------------------------------------------------------------------


Version 0.5.2 (10/12/2011)
=================
New Features
------------
* Background images (only jpz).
* Marks in the corner of squares (only jpz).
* Auto Save.

Bug Fixes / Improvements
------------------------
* Save button is grayed when no changes have been made (it used to change
  to Save As).
* Focus remains on grid when the clue list is clicked.
* Rebus entries are actually allowed to be any length.
* Long rebus entries are truncated and given tooltips.
* Rebus entries are displayed slightly larger.
* Fixed a crash on puzzles with non-linear words (e.g. Looking Glass).


----------------------------------------------------------------------


Version 0.5.1 (9/17/2011)
=========================
Bug Fixes / Improvements
------------------------
* Improved diagramless navigation.
* Timer info is loaded/saved correctly for jpz.
* Formatted clues are saved correctly for XPF.
* Scripts that have any kind of error when loading are disabled.


----------------------------------------------------------------------


Version 0.5 (9/9/2011)
======================
New Features
------------
* Added a preference to start the timer when a puzzle is opened.
* Added a Delete Puzzle button.
* Read/write support for XPF.
* Read/write support for jpz.
* Partial read support for ipuz.
* Script updater / package manager.

* Better printing layout (could still be better and faster).

* Support for more puzzle features:
    * Diagonal clues
    * Clues made up of arbitrary sets of squares
    * Empty squares
    * Rebus entries with unlimited length

Bug Fixes
---------
* Lua errors result in the script being disabled.
* Fixed problems with puzzles being wrongly reported
  as incorrect.


----------------------------------------------------------------------


Version 0.4 development
=======================
New Features
------------
* Embedded lua for scripting.  Included scripts:
    * Puzzle downloading
    * Puzzle database
    * "Swap across and down" written in lua
    * OneAcross.com search
    * Wikipedia.com search
    * Non-native file import
    * Try all letters (A-Z) in a blank square of the current word.

* User is now prompted before revealing the entire grid.
* Shift+8 (asterisk) toggles a circle in the current square.
* Check / Reveal can use a user-defined selection of squares.

Bug Fixes
---------
* .puz files now correctly use the full Windows-1252 code page instead of
  latin 1.
* Fixed a bug with obscure rebus entry behavior resulting in incorrectly
  reported solved / unsolved puzzles.
* "Move to next blank" will now wrap to the first blank in the word if the
  last blank has been entered (i.e. A-A-T; the user enters "I" as the 4th
  letter, and the cursor moves to the second letter).
* Spacebar always deletes the current letter and moves to the next square
  regardless of preferences.
* XWord correctly loads puzzles using webdings for rebus entries (entering
  webdings symbols in the grid is not currently supported, though XWord
  can display symbols in a puzzle previously saved by Across Lite).

Misc
----
* Revamped build system using premake4 to generate platform-specific
  project files and makefiles.
  Projects for MSVC2006, and MSVC2008.
  Makefile for linux.
* Removed conversion dialog.


----------------------------------------------------------------------


Version 0.3.1 (12/11/2009)
==========================
* Fixed "Move to next blank" bug (really, this time I mean it)


----------------------------------------------------------------------


Version 0.3 (12/11/2009)
========================
New Features
------------
* If a puzzle file is corrupt and not entirely unrecoverable, an option is
  given to load the puzzle anyway.
* A dialog supporting batch conversion of puzzle files (from .txt to .puz
  and vice-versa) has been added under the tools menu.  If the
  -convert [-c] command line option is given when XWord is invoked, only
  the conversion dialog appears.  Closing the dialog ends the program.
  Files to convert can still be given as command line parameters, and other
  options (-directory [-d], -output-files [-o], -overwrite [-w]) work as
  expected. If XWord is invoked with -c, conversion begins immediately.
* Added a clue prompt format preference.
* Added separate preferences for grid clue number and text fonts.
* Added an option to tweak the proprtion of text size to clue number size
  in the grid.
* Rudimentary printing.
* Added "Swap across and down" under the tools menu for a new perspective
  on your crossword puzzle!

Bug Fixes
---------
* A correctly filled grid with revealed letters is noted as solved instead
  of displaying the message
    "The puzzle is completely filled, but some letters are incorrect."
* "Reveal letter" and "Reveal word" now reveal squares if they are blank
  or incorrect (previously they only revealed incorrect squares).
* The "Check while typing" option does not attempt to check squares if the
  puzzle is scrambled.  If this option is selected when the puzzle is
  unscrambled, the puzzle is immediately checked for incorrect letters.
* "Move to next square" and "Move to next blank" preferences under the
  "After entering a letter" section now work as expected.
* All preferences now update immediately when then are changed (so you can
  preview the changes in the main window).
* Removed the pencil color setting until a pencil is implemented.
* Notepad window can *always* be opened or closed.
* Loading and saving the LTIM section of puz files is aware of whether
  or not the timer is running.

Misc
----
* Changed the name of the "Options" dialog to "Preferences".
* The default "crossing clue background color" is now white.


----------------------------------------------------------------------


Version 0.2.1 (bug fix) (10/14/2009)
====================================
* Fixed timer starting/stopping issues when activating and deactivating
  XWord.
* Notepad window can be closed.
* The grid is replaced with a "(Paused)" message if the timer is running
  when XWord is deactivated.


----------------------------------------------------------------------


Version 0.2 (10/09/2009)
========================
* Rebus puzzles can now be solved!
* Scrambled puzzles are checked once the puzzle is completely filled.
* "Corrupted" .puz files with the wrong version string are automatically
  tested with other version strings.
* Puzzle information sections (clue prompt, author, title, and copyright)
  are now centered instead of left- and top-aligned.  This will be
  configurable in the future.
* Puzzle title is displayed in the title bar.

File Locations
--------------
* XWord will now search a set of directories for its files rather than
  expecting all the files to be in the executable directory.  Directories
  searched depend both on the OS used and the directory that XWord is run
  from.
* Portable mode: if a file named "config.ini" is found in the executable's
  directory (linux or windows), XWord will write all config settings to
  that file and will not touch any other folders.

Config
------
* Added configuration settings!
* Created a very basic options dialog with wxFormBuilder
* Config file is named "config.ini" on windows and ".xword" on linux

Windows-specific
----------------
* Removed icons from menus, as menu icon rendering is not implemented well
  in wxWidgets under Vista


----------------------------------------------------------------------


Version 0.1 (07/31/2009)
========================
* Initial release
