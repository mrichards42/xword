XWord for Crossword Solver Users
--------------------------------

XWord supports [Crossword Solver](http://www.crosswordsolver.info/)'s
.jpz format, including the various gimmicks it allows.  XWord provides a
significantly different solving experience (based on Crossword Solver 1.2).

#### [Grid navigation](navigation.html) ####

- Crossword Solver navigates the grid similarly to Across Lite, but doesn't
  respond to `HOME` and `END` keys.  
  Right click always moves the cursor, and only switches the direction when
  the user clicks on the focused square (left click functions identically).  
  Crossword Solver does not provide any customizability in terms of grid
  navigation.  
  Crossword Solver does not move the cursor to the first blank when
  switching words.
- XWord has roughly the same navigation capabilites and
  customizability as [Across Lite](acrosslite.html).


#### [Diagramless](diagramless.html) puzzles ####

- Crossword Solver displays black squares in diagramless puzzles as white
  squares filled with ":".
- XWord allows interactive solving of diagramless puzzles.

#### [Layout](layout.html) ####

- Crossword Solver has a single layout that is not resizable.  Puzzles in
  jpz format can customize the user iterface (e.g. supply a different
  message when the puzzle is complete, disable reveal buttons).
- XWord allows the user to position GUI components anywhere and resize them
  to the user's liking.  XWord does not support loading or saving interface
  information in puzzle files.

#### [Check and reveal](check.html) ####

- Crossword Solver allows the user to check the whole grid, or reveal the
  current letter, word, or entire grid.
- XWord allows the user to check or reveal the current letter, word, or
  entire grid, and also allows the user to check or reveal a section of
  the grid.

#### Pencil ####

- XWord does not allow users to pencil-in answers.
