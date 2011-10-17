return {
    -- The current version of XWord
    xword = {
        name = "XWord", version = "0.5",
        download = "http://sourceforge.net/projects/wx-xword/files/Binary",
    },

    -- Lua scripts

    {
        name = "Clue Jump", version = "1.1",
        packagename = "cluejump",
        description = "Jump to the clue referenced in the current clue.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/cluejump_1.1.tar.gz",
        requires = "0.4",
    },
    {
        name = "Swap across and down", version = "1.0",
        packagename = "swap",
        description = "Swap across and down words and clues in the grid for a new perspective.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/swap_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "Try A-Z", version = "1.0",
        packagename = "tryaz",
        description = "Presents a dialog with all possible letters filled in for the current letter.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/tryaz_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "Puzzle Downloader", version = "1.0",
        packagename = "download",
        description = "Puzzle downloader and organizer.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/download_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "Puzzle navigator", version = "1.0",
        packagename = "navigator",
        description = "Move between puzzles in the same directory.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/navigator_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "Next blank", version = "1.0",
        packagename = "nextblank",
        description = "Move to the next blank square in the puzzle.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/nextblank_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "Puzzle import", version = "1.0",
        packagename = "import",
        description = "Import non-native puzzle types:\
* UClick XML\
",
        download = "http://www.sourceforge.net/wx-xword/files/lua/import_1.0.tar.gz",
        requires = "0.4",
    },
    {
        name = "XWord Debug", version = "0.4",
        packagename = "xworddebug",
        description = "Debug and lua script development package for XWord.",
        download = "http://www.sourceforge.net/wx-xword/files/lua/xworddebug_0.4.tar.gz",
        requires = "0.4",
    },

    -- Test package
-- --[[
    {
        name = "Hello World",
        version = "1.1",
        packagename = "hello",
        description = "Test package for install/uninstall (2)",
        requires = "0.4",
        download = "file:///D:/c++/xword/test/hello.zip",
    }
--]]
--[[
    {
        name = "Hello World",
        version = "1.0",
        packagename = "hello",
        description = "Test package for install/uninstall",
        requires = "0.4",
        download = "file:///D:/c++/xword/test/test.zip",
    }
--]]
}
