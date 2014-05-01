-- ============================================================================
-- debug.lua
--     Provides the xword.debug function for lua debug output
-- ============================================================================
local tablex = require 'pl.tablex'

local sep = '    '
local function get_message(...)
    local args = { n = select("#", ...), ... }
    if type(args[1]) == 'string' then
        -- Get the number of format params are in the first argument
        local _, field_count = args[1]:gsub('%%%%', ''):gsub('%%', '')
        -- Get the difference between this and the number of arguments
        local extra_args = args.n - 1 - field_count
        -- Format the string
        local success, msg = pcall(string.format, ...)
        if success then
            -- Add arguments that did not get incorporated
            if extra_args > 0 then
                local t = {msg}
                for i=field_count+2,args.n do
                    table.insert(t, tostring(args[i]))
                end
                return table.concat(t, sep)
            else
                return msg
            end
        end
    end
    return table.concat(tablex.imap(tostring, args), sep)
end

-- The dialog
local dlg
local function get_dialog()
    if not dlg then
        dlg = require'xworddebug.wxFB'.DebugDialog(xword.frame)
        function dlg:add_message(msg)
            self.text:AppendText(msg .. '\n')
        end
    end
    return dlg
end


-- Replace print with a debug dialog
local msg = false -- Recursion control
function xword.debug(...)
    if msg then
        table.insert(msg, get_message(...))
        return
    else
        msg = {}
    end
    local dlg = get_dialog()
    if msg then
        if #msg > 0 then
            dlg:add_message(table.concat(msg, '\n'))
        end
        msg = false
    end
    dlg:add_message(get_message(...))
    dlg:Show()
end
old_print = print
print = xword.debug

-- Set task debug handler
local task = require 'task'
task.debug_handler = xword.debug