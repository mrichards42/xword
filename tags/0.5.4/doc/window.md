The Main Window
---------------

<!-- Fancy image map -->

<div class="image-map"
     onmouseover="this.className='image-map focused'"
     onmouseout="this.className='image-map'"
     style="width: 587px;">

<div style="position: relative;">
<img src="images/window.png"/>

<!-- The rectangles -->
<div style="position:absolute; left:8px; top:31px; width:31px; height:17px;">
    <a href="#file_menu" title="File Menu"></a>
</div>
<div style="position:absolute; left:42px; top:31px; width:34px; height:17px;">
    <a href="#view_menu" title="View Menu"></a>
</div>
<div style="position:absolute; left:80px; top:31px; width:54px; height:17px;">
    <a href="#solution_menu" title="Solution Menu"></a>
</div>
<div style="position:absolute; left:139px; top:31px; width:39px; height:17px;">
    <a href="#tools_menu" title="Tools Menu"></a>
</div>
<div style="position:absolute; left:182px; top:31px; width:34px; height:17px;">
    <a href="#help_menu" title="Help Menu"></a>
</div>
<div style="position:absolute; left:9px; top:52px; width:27px; height:29px;">
    <a href="#opening_puzzles" title="Open Puzzle"></a>
</div>
<div style="position:absolute; left:38px; top:52px; width:30px; height:29px;">
    <a href="#saving_puzzles" title="Save Puzzle"></a>
</div>
<div style="position:absolute; left:105px; top:52px; width:97px; height:29px;">
    <a href="#grid_size" title="Grid Size"></a>
</div>
<div style="position:absolute; left:206px; top:52px; width:97px; height:29px;">
    <a href="check.html" title="Check Solution"></a>
</div>
<div style="position:absolute; left:307px; top:52px; width:32px; height:29px;">
    <a href="layout.html" title="Edit Layout"></a>
</div>
<div style="position:absolute; left:341px; top:52px; width:32px; height:29px;">
    <a href="#notes" title="Notes"></a>
</div>
<div style="position:absolute; left:378px; top:52px; width:32px; height:29px;">
    <a href="solving.html#rebus_entries" title="Rebus Entries"></a>
</div>
<div style="position:absolute; left:418px; top:52px; width:32px; height:29px;">
    <a href="#timer" title="Timer"></a>
</div>
<div style="position:absolute; left:9px; top:83px; width:207px; height:458px;">
    <a href="#clue_lists" title="Clue Lists"></a>
</div>
<div style="position:absolute; left:221px; top:126px; width:358px; height:49px;">
    <a href="#clue_prompt" title="Clue Prompt"></a>
</div>
<div style="position:absolute; left:221px; top:83px; width:358px; height:38px;">
    <a href="#title_author_copyright" title="Title, Author, and Copyright"></a>
</div>
<div style="position:absolute; left:248px; top:325px; width:48px; height:25px;">
    <a href="check.html" title="Incorrect Letters"></a>
</div>
<div style="position:absolute; left:225px; top:440px; width:71px; height:25px;">
    <a href="check.html" title="Revealed Letters"></a>
</div>
<div style="position:absolute; left:478px; top:256px; width:25px; height:25px;">
    <a href="check.html" title="Checked Letters"></a>
</div>
<div style="position:absolute; left:294px; top:350px; width:25px; height:184px;">
    <a href="navigation.html" title="Grid Navigation"></a>
</div>
<div style="position:absolute; left:480px; top:544px; width:97px; height:19px;">
    <a href="#timer" title="Timer"></a>
</div>
<div style="position:absolute; left:8px; top:544px; width:468px; height:19px;">
    <a href="#status_bar" title="Status Bar"></a>
</div>
</div>

</div>


<!-- Information -->

### Panes ###

#### Clue Prompt ####

The clue prompt displays the currently focused clue in large text.  Clues with
text that exceeds the boundary of the clue prompt will be shrunken and wrapped
to fit.  If the text cannot be shrunken and wrapped sufficiently to read it,
the clue prompt will have a tooltip with the current clue.

#### Clue Lists ####

XWord supports unlimited clue lists, though the vast majority of puzzles will
have just Across and Down clues.  Clues with text that exceeds the boundary
of the clue list will be wrapped.  The currently focused clue is highlighted,
and the clue that crosses the focused letter is highlighted in a different
color.  Clicking on a clue in a clue list will make that clue the focused word.

#### Title, Author, Copyright ####

If a puzzle contains a title, the author's name, or a copyright, these will be
displayed in their own panes.

#### Notes ####

Notes are read-only and can contain simple xhtml formatting.  Show and hide the
notes window using the [`View`](#view_menu) menu, the toolbar buttons, or the
[context menu](layout.html#context_menu) or
[close button](layout.html#pane_buttons) on the pane.  If a puzzle has notes,
the toolbar icon will turn yellow.


### File Menu ###

The file menu contains menu items for opening, saving, and deleting puzzles,
printing, and the [preferences](preferences.html) dialog.

#### Opening Puzzles ####

Puzzles can be opened by selecting `Open` from the `File` menu, by clicking
on the `Open Puzzle` toolbar button, by drag-and-drop, or by a puzzle
downloader [package](packages.html).

#### Saving Puzzles ####

Puzzles can be saved by selecting `Save` or `Save As` from the `File` menu,
or by clicking on the `Save Puzzle` toolbar button.  If no modifications have
been made to the puzzle, the `Save Puzzle` toolbar button will be disabled.
XWord can also be set to [auto save](preferences.html#auto_save) puzzles after
a certain amount of time.

#### Printing Puzzles ####

XWord prints puzzles on a single page, attempting to achieve the correct
balance between readable clues and a large grid.  Puzzles can be printed with
a blank grid, as currently solved, or with the solution.


### View Menu ###

The view menu contains grid sizing options and menu items related to adjusting
the [window layout](layout.html)

#### Grid Size ####

The grid can be zoomed in and out, or sized to fit the window.  If the grid
is zoomed in, scrollbars appear in the grid.  As the cursor is
[moved](navigation.html) around the grid, the currently focused area
automatically scrolls into view.


### Solution Menu ###

The solution menu allows the user to [check, reveal](check.html),
[erase](solving.html#erasing_letters), or [unscramble](#unscrambling) the grid,
and enter [multiple letters](solving.html#rebus_entries) in a square.

#### Unscrambling ####

Some .puz files come with a scrambled solution.  To unscramble the solution,
you need a 4-digit code.  Puzzles with scrambled solutions will have the
check and reveal solution menus and buttons disabled.  If a scrambled puzzle
has been filled in, XWord *will* alert the user if the puzzle is filled
correctly or incorrectly.


### Tools Menu ###

The tools menu allows the user to toggle the [timer](#timer) and install or
uninstall [packages](packages.html).  Packages typically add menu items to the
tools menu.

#### Timer ####

XWord includes a timer so you can keep track of how long it takes you to solve
puzzles.  The solving time is displayed in the bottom right corner in the
[status bar](#status_bar).  The timer can optionally be made to start when
a puzzle is opened (see [preferences](preferences.html#solving_preferences)).


### Help Menu ###

The help menu contains an about dialog, the [license](license.html), and links
to these help files.


### Status Bar ###

The status bar displays various information about the current puzzle, including
the following:

- The filename
- Solving [time](#timer)
- When the puzzle is filled in, a section of the status bar will display a
  message and change color depending on whether the puzzle is correct or not.
- Errors encountered in [packages](packages.html)