Entering Letters in the Grid
----------------------------

To enter a letter into a square, [navigate](navigation.html) to the square and
type a letter. The focused word is highlighted in one color, and the focused
square is highlighted in a different color.  Unless
[configured](preferences.html#solving_preferences) otherwise, the focused square
will move to the next square in the current word when a letter is typed.

XWord supports entering any printable character[^1] in a square, including
symbols, however certain puzzle formats (such as .puz) only support a limited
character set.


### Erasing Letters ###
- `BACKSPACE` erases the letter in the current square and moves the cursor
   backward one square.
- `DELETE` erases the letter in the current square and does not move
   the cursor.
- `SPACE` erases the letter in the current square and moves the cursor
   forward one square.
- The [`Solution`](window.html#solution_menu) menu contains two erasing items:
    - `Erase Grid` clears the grid and resets the timer.
    - `Erase Uncrossed Letters` erases letters in the current word that are
       not part of an already filled word.


### Rebus Entries ###

Some puzzles require multiple letters to be entered in a square.

1. Enter rebus mode by pressing `INSERT`, clicking the rebus toolbar icon, or
   selecting `Enter Multiple Letters` from the
   [`Solution`](window.html#solution_menu) menu.
2. An expandable text box will appear in which you can type multiple letters.
3. Press `INSERT` or `ENTER` to exit rebus mode (the cursor will become
   solid again).

For display, rebus entries longer than 4 characters are wrapped and entries
longer than 10 characters are truncated.  If the mouse is hovered over a rebus
entry, a tool tip will pop up and display the entire entry.

![Image of filling a rebus entry](images/rebus.png "Filling a rebus entry")


### Webdings Symbols ###

Across Lite allows users to enter symbols from the Webdings font.
If Webdings is installed, XWord will display these symbols, but it does not
support entering Webdings symbols.


### Circles ###

Typing an asterisk (\*) will toggle a circle in the current square.


[^1]: A few reserved characters are not allowed to be entered as text: <code>: . [ </code>and<code> *</code>