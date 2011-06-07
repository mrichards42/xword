-- version 0.4.0.12
return {
    version = "0.4.0.12",

    -- Lua scripts

        {
            name = "XWord lua", version = "0.4",
            kind = "script",
            download = "http://www.sourceforge.net/wx-xword/files/lua/xword_0.4.tar.gz",
            description = "XWord lua initialization scripts",
            requires = { {"XWord", "0.4"}, {"Lua", "5.1"}, {"luapuz", "0.4"}, {"SafeExec", "1.0"} },
            packagename = "xword"
        },
        {
            name = "Clue Jump",
            packagename = "cluejump",
            version = "1.1",
            kind = "script",
            description = "Jump to the clue referenced in the current clue.",
            requires = {
                { "XWord lua", "0.4", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/cluejump_1.1.tar.gz",
        },
        {
            name = "Swap across and down",
            packagename = "swap",
            version = "1.0",
            kind = "script",
            description = "Swap across and down words and clues in the grid for a new perspective.",
            requires = {
                { "XWord lua", "0.4", },
                { "Penlight", "0.7", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/swap_1.0.tar.gz",
        },
        {
            name = "Try A-Z",
            packagename = "tryaz",
            version = "1.0",
            kind = "script",
            description = "Presents a dialog with all possible letters filled in for the current letter.",
            requires = {
                { "XWord lua", "0.4", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/tryaz_1.0.tar.gz",
        },
        {
            name = "Puzzle Downloader",
            packagename = "download",
            version = "1.0",
            kind = "script",
            description = "Puzzle downloader and organizer.",
            requires = {
                { "XWord lua", "0.4"},
                { "wxluatask", "0.4", },
                { "LuaDate", "2.0.1", },
                { "LuaFileSystem", "1.4.2", },
                { "luacurl", "0.4", },
                { "luapuz", "0.4", },
                { "Serialize", "1.0", },
                { "Penlight", "0.7", },
                { "Puzzle database", "1.0", },
                { "Puzzle import", "1.0", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/download_1.0.tar.gz",
        },
        {
            name = "Puzzle navigator",
            packagename = "navigator",
            version = "1.0",
            kind = "script",
            description = "Move between puzzles in the same directory.",
            requires = {
                { "XWord lua", "0.4"},
                { "Puzzle import", "1.0"},
                { "Penlight", "0.7", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/navigator_1.0.tar.gz",
        },
        {
            name = "Next blank",
            packagename = "nextblank",
            version = "1.0",
            kind = "script",
            description = "Move to the next blank square in the puzzle.",
            requires = {
                { "XWord lua", "0.4"},
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/nextblank_1.0.tar.gz",
        },
        {
            name = "Puzzle import",
            packagename = "import",
            version = "1.0",
            kind = "script",
            description = "Import non-native puzzle types:\
    * UClick XML\
",
            requires = {
                { "XWord lua", "0.4", },
                { "luapuz", "0.4", },
                { "LuaExpat", "1.1.0", },
                { "Penlight", "0.7", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/import_1.0.tar.gz",
        },
        {
            name = "XWord Debug",
            packagename = "xworddebug",
            version = "0.4",
            kind = "script",
            description = "Debug and lua script development package for XWord.",
            requires = {
                { "XWord lua", "0.4", },
                { "Puzzle database", "1.0" },
                { "luapuz", "0.4", },
                { "Custom widgets", "0.4" },
                { "Penlight", "0.7", },
            },
            download = "http://www.sourceforge.net/wx-xword/files/lua/xworddebug_0.4.tar.gz",
        },

    -- Test package
-- --[[
    {
        name = "Hello World",
        version = "1.1",
        kind = "script",
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
        kind = "script",
        packagename = "hello",
        description = "Test package for install/uninstall",
        requires = "0.4",
        download = "file:///D:/c++/xword/test/test.zip",
    }
--]]
}