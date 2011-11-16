XWord Support for Puzzle Features
---------------------------------

| Feature                                       | puz | xpf | jpz | ipuz[^1] |
| --------------------------------------------- |:---:|:---:|:---:|:----:|
| Scrambled solution                            |  X  |     |     |      |
| [Rebus](solving.html#rebus_entries) entries   |  X  |  X  |  X  |  X   |
| Circled squares                               |  X  |  X  |  X  |  X   |
| Squares with colored or shaded backgrounds    |     |  X  |  X  |  X   |
| Background images                             |     |     |  X  |      |
| [Diagramless](diagramless.html) puzzles       |  X  |  X  |     |      |
| Diagonal words                                |     |  X  |  X  |      |
| Words in other directions                     |     |     |  X  |      |
| Clues with formatted text                     |     |  X  |  X  |  X   |
| Notes with formatted text                     |     |  X  |  X  |  X   |
| More than 2 clue lists                        |     |     |  X[^2] |  X   |
| Unconventional numbering schemes              |     |  X  |  X  |  X   |
| Unclued words                                 |     |  X  |  X  |      |

[^1]: XWord can load ipuz files but cannot write them.
[^2]: jpz format only allows 2 clue lists, but XWord supports unlimited clue
      lists.  XWord will read and write altered jpz files with extra clue
      lists, but [Crossword Solver](crosswordsolver.html) will refuse to load
      them.