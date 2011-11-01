XWord for Across Lite Users
---------------------------

XWord is intended to be as familiar as possible for users transitioning from
[Across Lite](http://www.litsoft.com/across/alite/download/).  Besides
supporting more formats (and more crosswordy-gimmicks), there are a few
subtle (and not so subtle) differences between XWord and Across lite (based on
Across Lite 2.0.5).

#### [Diagramless](diagramless.html) puzzles ####

- Across Lite gives the user an option to print diagramless puzzles or to
  display the black squares for interactive solving.
- XWord allows interactive solving of diagramless puzzles.

#### Long clues ####

- Long clues in Across Lite are truncated or displayed in a very small
  font size.
- Long clues in XWord are wrapped so that they display in a normal font
  size.  In case a clue is especially long, a tool tip is shown if the
  user hovers the mouse over the main clue prompt.

#### [Layout](layout.html) ####

- Across Lite provides 9 layouts with some limited customizability.  Across
  Lite gives users the option of customizing the toolbar.
- XWord allows the user to position GUI components anywhere.  While this
  makes any layout possible, it can sometimes be difficult to get
  components to go where you want them to ("Argh, I moved the title pane, and
  now I can't get it back where it was!).  
  To mitigate some of the potential frustration with manually arranging
  panes, XWord ships with a a set of default layouts that approximately
  mimic the familiar Across Lite layouts.

#### [Check and reveal](check.html) ####

- Across Lite allows the user to check or reveal the current letter, word,
  or entire grid.
- XWord also allows the user to check or reveal a section of the grid.

#### [Grid navigation](navigation.html) ####

- In Across Lite, when the focused clue is Down, `SHIFT+DOWN` moves the
  cursor to the *next* Down clue.  
  Essentially this behavior is the same as `TAB`.  
  If the user clicks on a clue in Across Lite, focus shifts to the clue list.
- In XWord, when the focused clue is Down, `SHIFT+DOWN` moves the cursor
  to the Down clue *below* the current one.  
  This behavior is similar to pressing `SHIFT+ACROSS` when the focused
  clue is Across.  
  If the user clicks on a clue in XWord, focus remains on the grid.

#### Pencil ####

- XWord does not allow users to pencil-in answers.

#### Webdings symbols ####

- XWord will display webdings symbols, but does not allow the user to input
  webdings symbols.

#### Clipboard ####

- XWord does not allow users to copy and paste text in the grid.

#### [Notepad](window.html#notes) ####

- XWord does not allow the user to type in the notepad.  Additionally,
  the notepad can contain an unlimited number of characters and xhtml
  formatting.

#### "Corrupt" puzzles ####

- XWord will attempt to load all .puz files.  If the puzzle is internally
  consistent (the clues match up with the grid) XWord will load the puzzle
  without complaint.