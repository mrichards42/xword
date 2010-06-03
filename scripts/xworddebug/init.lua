local function openpuz()
    --open a puzzle
    print("Hello world")
    xword.frame:LoadPuzzle([[D:\C++\XWord\test_files\2005 - 10 - 28 - That's Rich.puz]])
    p = xword.frame.Puzzle
    print("Puzzle: "..p:GetTitle())
end

local function init()
    -- Add a "debug" submenu to the tools menu
    xword.frame:FindOrCreateMenu({'Tools', 'Debug'})
end

init()

-- Require the rest of the debug directory
xword.requiredir('xworddebug')