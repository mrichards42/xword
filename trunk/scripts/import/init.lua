-- ============================================================================
-- Import
--     A somewhat complex XWord add-on that implements an Import menu for
--     foreign crossword puzzle types.

--     Each recognized type requires a separate lua script that must be placed
--     in the import directory.  In order to add a file type to the Import
--     menu, the script should call import.addHandler() described below.
-- ============================================================================

-- The package table
import = {}


-- ============================================================================
-- Import menu
-- ============================================================================

-- Create the import menu and return it
local function createMenu()
    assert(wx and xword and xword.frame)

    -- Add an Import submenu to the file menu
    local importMenu, isnew = xword.frame:GetMenu('File', 'Import')
    if not isnew then return importMenu end

    -- Find the Close menu so that we can insert the import menu ahead of it
    local fileMenu = xword.frame:GetMenu('File')
    local index = xword.findMenuItemIndex(fileMenu, 'Close')

    -- The next item is a separator
    index = index + 1

    -- Insert a separator for the Import menu
    -- This will show up belowe the Import menu
    fileMenu:InsertSeparator(index + 1)

    -- Insert the Import menu into the correct place
    -- wxMenuitem arguments: (parent, id, title, help text, item kind, submenu)
    local item = fileMenu:FindItemByPosition(fileMenu:GetMenuItemCount() - 1)
    fileMenu:Insert(index + 1, fileMenu:Remove(item))
    return importMenu
end

-- Return the import menu (create if necessary)
local importMenu
function import.getMenu()
    if importMenu then return importMenu end
    -- We can't do anything if XWord doesn't have a frame
    if not xword or not xword.frame then return end
    importMenu = createMenu()
    return importMenu
end

-- Add an item to the import menu
local function addToMenu(loadfunc, ext, label)
    assert(xword and xword.frame)

    -- Add to the menu
    local menuItem = import.getMenu():Append(wx.wxID_ANY, label)

    -- Make the wildcard string for wxWidgets
    local wildcard =
        label.." Format (*."..ext..")|*."..ext.."|All Files (*.*)|*.*"

    -- Connect the menu event
    xword.frame:Connect(menuItem:GetId(),
                        wx.wxEVT_COMMAND_MENU_SELECTED,
        function(evt)
            local filename = wx.wxFileSelector(
                'Select a file to import',
                '', '', '', wildcard,
                wx.wxFD_OPEN + wx.wxFD_FILE_MUST_EXIST,
                xword.frame
            )

            if filename ~= '' then
                xword.frame:LoadPuzzle(filename, loadfunc)
            end
        end
    )
end


--[[
handler table format:
{
    {
        ext = extension,
        load = function(puz, filename) end,
        desc = description/label
    },
    [...]
}
]]
import.handlers = {
}

-- ============================================================================
-- Add a handler to the import handlers table.

-- loadfunc should take a puzzle and filename as parameteters.  It should
-- return true on succes, raise an error on failure.

-- ============================================================================
function import.addHandler(loadfunc, ext, label)
    assert(label and ext and loadfunc)
    ext = ext:lower()

    -- Add to the handlers table
    table.insert(import.handlers,
                 { desc = label, ext = ext, load = loadfunc })
end

local tablex = require 'pl.tablex'
local path = require 'pl.path'
-- Try all handlers on a puzzle.
function import.load(p, filename)
     -- Extension without the dot
    local ext = path.extension(filename):lower():sub(2)

    -- Try all handlers, but prefer those with a matching extension
    local handlers = tablex.filter(import.handlers,
                                   function (h) return h.ext == ext end)
    table.insert(handlers, {}) -- this empty table will be used for Puzzle.Load()
    tablex.insertvalues(
        handlers,
        tablex.filter(import.handlers,
                      function (h) return h.ext ~= ext end)
    )

    local closest_err = '' -- Error from an extension that matches
    for _, handler in ipairs(handlers) do
        p:Clear()
        local success, err
        if handler.load then
            success, err = pcall(handler.load, p, filename)
        else
            success, err = pcall(Puzzle.Load, p, filename)
        end
        if success then
            return true
        elseif handler.ext == ext then
            closest_err = err
        end
    end
    error(closest_err)
end


function import.init()
    require('import.uclick')

    -- Add handlers to the menu
    if xword and xword.frame then
        for _, handler in ipairs(import.handlers) do
            addToMenu(handler.load, handler.ext, handler.desc)
        end
    end
end

function import.uninit()
    -- Remove the separator under the menu
    local fileMenu = xword.frame:GetMenu('File')
    local index = xword.findMenuItemIndex(fileMenu, 'Import')
    fileMenu:Destroy(fileMenu:FindItemByPosition(index + 1))
    xword.frame:RemoveMenu('File', 'Import')
    importMenu = nil
end

return import
