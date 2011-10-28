Overview of the XWord main window
---------------------------------

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
    <a href="#opening_puzzle" title="Open Puzzle"></a>
</div>
<div style="position:absolute; left:38px; top:52px; width:30px; height:29px;">
    <a href="#saving_puzzle" title="Save Puzzle"></a>
</div>
<div style="position:absolute; left:70px; top:52px; width:30px; height:29px;">
    <a href="#file_menu" title="Delete Puzzle"></a>
</div>
<div style="position:absolute; left:105px; top:52px; width:97px; height:29px;">
    <a href="#grid_size" title="Grid Size"></a>
</div>
<div style="position:absolute; left:206px; top:52px; width:97px; height:29px;">
    <a href="#check" title="Check Solution"></a>
</div>
<div style="position:absolute; left:307px; top:52px; width:32px; height:29px;">
    <a href="layout.html" title="Edit Layout"></a>
</div>
<div style="position:absolute; left:341px; top:52px; width:32px; height:29px;">
    <a href="#notes" title="Notes"></a>
</div>
<div style="position:absolute; left:380px; top:52px; width:32px; height:29px;">
    <a href="#timer" title="Timer"></a>
</div>
<div style="position:absolute; left:9px; top:83px; width:207px; height:458px;">
    <a href="#clue_list" title="Clue Lists"></a>
</div>
<div style="position:absolute; left:221px; top:126px; width:358px; height:49px;">
    <a href="#clue_prompt" title="Clue Prompt"></a>
</div>
<div style="position:absolute; left:221px; top:83px; width:358px; height:38px;">
    <a href="#metadata" title="Title, Author, and Copyright"></a>
</div>
<div style="position:absolute; left:248px; top:325px; width:48px; height:25px;">
    <a href="#incorrect_letter" title="Incorrect Letters"></a>
</div>
<div style="position:absolute; left:225px; top:440px; width:71px; height:25px;">
    <a href="#revealed_letter" title="Revealed Letters"></a>
</div>
<div style="position:absolute; left:478px; top:256px; width:25px; height:25px;">
    <a href="#checked letters" title="Checked Letters"></a>
</div>
<div style="position:absolute; left:294px; top:350px; width:25px; height:184px;">
    <a href="#navigation" title="Grid Navigation"></a>
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

### File Menu ###

The file menu contains menu items for opening, saving, and deleting puzzles,
printing, and the [preferences](preferences.html) dialog.

#### Opening Puzzles ####

Puzzles can be opened through a dialog, by drag-and-drop, or by a puzzle
downloader [package](packages.html).


### View Menu ###

The view menu contains grid sizing options and menu items related to adjusting
the [window layout](layout.html)

#### Grid Size ####

The grid can be zoomed in and out, or sized to fit the window.  If the grid
is zoomed in, scrollbars appear in the grid.  As the cursor is
[moved](navigation.html) around the grid, the currently focused area
automatically scrolls into view.


### Solution Menu ###

#### Checking and Revealing Answers ####


### Tools Menu ###

The tools menu allows the user to toggle the [timer](#timer) and install or uninstall
[extensions](packages.html).  Extensions typically add menu items to the tools
menu.

#### Timer ####

XWord includes a timer so you can keep track of how long it takes you to solve
puzzles.  The solving time is displayed in the bottom right corner in the
[status bar](#status_bar).  The timer can optionally be made to start when
a puzzle is opened (see [preferences](preferences.html#solving_preferences)).


### Help Menu ###

The help menu contains an about dialog, the [license](license.html), and links
to these help files.
