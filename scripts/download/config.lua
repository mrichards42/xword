-- Save the current directory for relative require
local _R = (string.match(..., '^.+%.') or ... .. '.')

local path = require 'pl.path'
local tablex = require 'pl.tablex'
local stringx = require 'pl.stringx'
local serialize = require 'serialize'
local os = require 'os'
local date = require 'date'

-- ----------------------------------------------------------------------------
-- cURL string -> enum conversion
-- ----------------------------------------------------------------------------
local curl = require 'luacurl'
local _curl_to_config = {}
local _curl_from_config = {}
for k, v in pairs(curl) do
    if k:sub(1,4) == "OPT_" then
        k = k:sub(5)
        _curl_to_config[v] = k
        _curl_from_config[k] = v
    end
end

-- Convert a table with numeric keys to use curl options
local function curl_to_config(opts)
    if not opts then return end
    local ret = {}
    for id, value in pairs(opts) do
        pcall(function()
            ret[_curl_to_config[id]] = value
        end)
    end
    return ret
end

-- Convert a table with curl options to numeric keys
local function curl_from_config(opts)
    if not opts then return end
    local ret = {}
    for id, value in pairs(opts) do
        if type(id) == 'string' then
            pcall(function()
                ret[_curl_from_config[id]] = value
            end)
        else
            ret[id] = value
        end
    end
    return ret
end


-- ----------------------------------------------------------------------------
-- Default configuration
-- ----------------------------------------------------------------------------
local config = {}
config.puzzle_directory = path.join(xword.userdatadir, "puzzles")
config.separate_directories = true
config.auto_download = 0
config.default_view = "Day"
config.previous_view = {}
config.styles = {
    error = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(180, 0, 0) },
    missing = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(wx.wxBLUE) },
    downloaded = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(128, 0, 128) },
    progress = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(34, 139, 34) },
    complete = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(34, 139, 34) },
    unknown = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(wx.wxBLACK) },
}
config.styles.progress.font.Weight = wx.wxFONTWEIGHT_BOLD

local default_disabled = {
    "NY Times Premium",
    "NY Times PDF",
    "Newsday",
    "USA Today",
    "Universal",
    "Matt Gaffney's Weekly Crossword Contest",
    "Matt Gaffney's Daily Crossword",
    "Brendan Emmett Quigley",
    "I Swear",
    "Washington Post Puzzler"
}

-- Keep a list of original keys to the config table
local config_keys = tablex.keys(config)


-- ----------------------------------------------------------------------------
-- Helpers
-- ----------------------------------------------------------------------------

-- Recursively update keys from t1 with values from t2
local function deep_update(t1, t2)
    if type(t1) ~= 'table' or type(t2) ~= 'table' then return end
    -- Set nil values
    for _, k in ipairs(t2._nil or {}) do
        t1[k] = nil
    end
    t2._nil = nil
    -- Copy values
    for k, v in pairs(t2) do
        if type(t1[k]) == 'table' then
            deep_update(t1[k], t2[k])
        else
            t1[k] = v
        end
    end
end

-- Load the SourceList config
local function load_sources_config(data)
    local sources = require(_R .. 'sources')
    -- Added
    for _, src in ipairs(data.added or {}) do
        src.curlopts = curl_from_config(src.curlopts)
        sources:insert(src)
    end
    -- Disabled
    for _, id in ipairs(data.disabled or default_disabled) do
        local src = sources[id]
        if src then
            src.disabled = true
        end
    end
    -- Changed
    for k, changes in pairs(data.changed or {}) do
        local src = sources[k]
        if src then
            deep_update(src, changes)
            -- Turn curl strings into numbers
            src.curlopts = curl_from_config(src.curlopts)
        end
    end
    -- Place sources in the order specified
    local oldorder = sources._order
    sources._order = {}
    for _, id in ipairs(data.order or {}) do
        for i, oldid in ipairs(oldorder) do
            if oldid == id then
                table.insert(sources._order, table.remove(oldorder, i))
                break
            end
        end
    end
    -- Add the remaining (unordered) sources in their default order
    for _, id in ipairs(oldorder) do
        table.insert(sources._order, id)
    end
end

-- Return differences between two tables, optionally ignoring some keys
-- a is the original, b is the new table
local function get_changes(original, new, ...)
    local ignored = tablex.makeset({...})
    local changes = { _nil = {} } -- _nil is a table of keys with nil values
    -- Check for removed / nil values
    for k,_ in pairs(original) do
        if new[k] == nil then
            table.insert(changes._nil, k)
        end
    end
    if #changes._nil == 0 then changes._nil = nil end
    -- Check for changed values
    for k,v in pairs(new) do
        if original[k] ~= v and not ignored[k] then
            if type(original[k]) == 'table' and type(new[k]) == 'table' then
                -- tables should be checked recursively
                changes[k] = get_changes(original[k], new[k])
            else
                changes[k] = tablex.deepcopy(v)
            end
        end
    end
    -- Only return if there are changes
    if next(changes) ~= nil then
        return changes
    end
end

-- Return the sources section of the config file
local function get_sources_config()
    local sources = require(_R .. 'sources')
    local data = {
        disabled = {},
        added = {},
        changed = {},
        order = sources._order
    }
    -- Disabled sources
    for id, src in pairs(sources or {}) do
        if src.disabled then
            table.insert(data.disabled, id)
        end
    end
    -- Figure out what has changed from the defaults
    local defaults = sources.get_default_sources()
    for _, src in sources:iterall() do
        local default = defaults[src.id]
        if not default then
            -- If this source doesn't exist in the defaults, the user
            -- added it
            local p = tablex.deepcopy(src)
            p.curlopts = curl_to_config(p.curlopts)
            table.insert(data.added, p)
        else
            -- Get changes (ignoring the 'disabled' key)
            local changes = get_changes(default, src, 'disabled')
            data.changed[src.id] = changes
            -- Turn curl options into strings
            if changes then
                changes.curlopts = curl_to_config(changes.curlopts)
            end
        end
    end
    return data
end

-- ----------------------------------------------------------------------------
-- Public functions
-- ----------------------------------------------------------------------------

function config.get_config_filename()
    return path.join(xword.configdir, 'download', 'config.lua')
end

local LOAD_ERROR = false
function config.load()
    local data, err = serialize.loadfile(config.get_config_filename())
    -- Check for errors
    if not data and path.isfile(config.get_config_filename()) then
        local msg =
            "Error while loading Puzzle Downloader configuration file.\n" ..
            "Would you like to reset Puzzle Downloader settings?"
        if type(err) == 'string' and #err > 0 then
            msg = msg .. "\n\nDetails:\n" .. err
        end
        local rc = wx.wxMessageBox(msg, "XWord Error", wx.wxYES_NO + wx.wxICON_ERROR)
        if rc == wx.wxYES then
            os.remove(config.get_config_filename())
        else
            LOAD_ERROR = true
        end
    end
    if type(data) ~= 'table' then return end
    -- Update the configuration values
    for _, k in ipairs(config_keys) do
        if k ~= 'styles' then -- we need special processing for colors/fonts
            if type(config[k]) == 'table' then
                deep_update(config[k], data[k])
            else
                config[k] = data[k]
            end
        end
    end
    -- Create dates from strings
    for _, k in ipairs({'start_date', 'end_date'}) do
        if type(config.previous_view[k]) == 'string' then
            config.previous_view[k] = date(config.previous_view[k])
        end
    end
    -- Create fonts and colors from strings
    for k, style in pairs(data.styles) do
        local config_style = config.styles[k]
        if config_style then
            if type(style.font) == 'string' then
                config_style.font:SetNativeFontInfo(style.font)
            end
            if type(style.color) == 'string' then
                config_style.color:Set(style.color)
            end
        end
    end
    -- Update sources config
    load_sources_config(data)
end

function config.save()
    if LOAD_ERROR then
        xword.Message(
            "Download configuration settings will not be saved due to errors when loading the file.\n" ..
            "Please fix errors in the file, or delete it: " .. config.get_config_filename()
        )
        return
    end
    -- Copy the configuration values to a data table
    local data = {}
    for _, k in ipairs(config_keys) do
        data[k] = tablex.deepcopy(config[k])
    end
    -- Turn dates into strings
    for _, k in ipairs({'start_date', 'end_date'}) do
        if config.previous_view[k] then
            data.previous_view[k] = config.previous_view[k]:fmt("%m/%d/%Y")
        end
    end
    -- Turn fonts and colors into strings
    for k, style in pairs(config.styles) do
        data.styles[k] = { font = style.font:GetNativeFontInfoDesc(),
                           color = style.color:GetAsString() }
    end
    -- Get the sources configuration data
    tablex.update(data, get_sources_config())
    -- Save to a file
    serialize.dump(data, config.get_config_filename())
end


if true then
return config
end

-- TODO: Move this somewhere else
-- ============================================================================
-- GUI
-- ============================================================================

local ArrowButton = require 'download.gui.arrow_button'
local TextButton = require 'download.gui.text_button'
local BmpButton = require 'download.gui.bmp_button'
local bmp = require 'download.gui.bmp'
local ScrolledPanel = require 'download.gui.scrolled_panel'


-- ----------------------------------------------------------------------------
-- Advanced Options / New source dialog
-- ----------------------------------------------------------------------------
require 'download.config_gui'

function show_advanced_options_dialog(parent, puzzle, title)
    local dlg = advanced_options_dialog(parent, title)

    -- Create a new puzzle if needed
    if puzzle == nil or not puzzle.name then
        puzzle = { name = '', filename = '', url = '',
                   days = { false, false, false, false, false, false, false} }
    end

    -- Reset button event
    dlg.reset_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local p = download.get_default_puzzles()[puzzle.id]
        if p then
            dlg:set_puzzle(p)
        end
    end)

    -- Set puzzle values into the dialog
    function dlg:set_puzzle(p)
        local panel = dlg.panel
        -- Basic options
        panel.name.Value = p.name
        panel.filename.Value = p.filename
        panel.url.Value = p.url
        panel.directoryname.Value = p.directoryname or download.sanitize_name(p.name)
        for i, ctrl in ipairs(panel.days) do
            ctrl.Value = p.days[i]
        end
        --cURL options
        panel.curlopts:clear()
        for k, v in pairs(p.curlopts or {}) do
            if type(v) == 'string' then
                panel.curlopts:add_option(k, v)
            end
        end
        -- Custom fields
        panel.fields:clear()
        for _, name in ipairs(p.fields or {}) do
            panel.fields:add_field(name)
        end
        -- Custom function
        panel.custom_function.Value = p.func or ''
    end

    -- Transfer from the dialog to the puzzle
    local function apply(p)
        local panel = dlg.panel
        -- Basic options
        p.name = panel.name.Value
        p.filename = panel.filename.Value
        p.url = panel.url.Value
        if panel.directoryname.Value ~= download.sanitize_name(p.name) then
            p.directoryname = panel.directoryname.Value
        else
            p.directoryname = nil
        end
        for i, ctrl in ipairs(panel.days) do
            p.days[i] = ctrl.Value
        end
        -- cURL options
        if not p.curlopts then
            p.curlopts = {}
        end
        tablex.clear(p.curlopts)
        local opts_table = panel.curlopts.opts
        for _, ctrls in ipairs(opts_table) do
            local opt, value = unpack(ctrls)
            -- Find the option as curl.OPT_[???]
            opt = curl['OPT_' .. opt.StringSelection]
            -- Set the option
            p.curlopts[opt] = value.Value
        end
        if #opts_table == 0 then
            p.curlopts = nil
        end
        -- Custom fields
        if not p.fields then
            p.fields = {}
        end
        tablex.clear(p.fields)
        local fields_table = panel.fields.fields
        for _, ctrl in ipairs(fields_table) do
            local value = ctrl.Value
            if value ~= '' then
                table.insert(p.fields, value)
            end
        end
        if #p.fields == 0 then
            p.fields = nil
        end
        -- Custom function
        local value = panel.custom_function.Value
        if value:match('^%s*$') then
            p.func = nil
        else
            p.func = value
        end
    end

    dlg:set_puzzle(puzzle)

    if dlg:ShowModal() == wx.wxID_OK then
        apply(puzzle)
        dlg:Destroy()
        return puzzle
    end
    dlg:Destroy()
end

-- ----------------------------------------------------------------------------
-- Config dialog
-- ----------------------------------------------------------------------------

local get_download_options_panel

function download.get_config_panel(parent)
    local panel = config_panel(parent)

    -- Puzzle sources
    -- ----------------------

    local puzzles = {}
    local puzzle_list = panel.sources.list
    local set_selection

    -- Update the puzzle sources list
    local function update_sources()
        -- Make a list of the names
        local names = {}
        for key, puzzle in puzzles:iterall() do
            table.insert(names, puzzle.name)
        end
        -- Reset the selection after we change the list
        local selection = panel.sources.list.Selection
        puzzle_list:Set(names)
        if selection == -1 then selection = 0 end
        set_selection(selection)
    end

    -- Is this a puzzle that the user added?
    local function is_user_puzzle(puzzle)
        local id
        if type(puzzle) == 'table' then
            id = puzzle.id
        else
            id = puzzle
        end
        return download.get_default_puzzles()[id] == nil
    end

    -- Button events
    -- ----------------------
    local buttons = panel.sources.buttons

    -- Move up in the list
    buttons.up:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 1 then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx-1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(puzzles[id].name, idx - 2)
            set_selection(idx - 2)
        end
    end)

    -- Move down in the list
    buttons.down:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx < #puzzles._order then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx+1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(puzzles[id].name, idx)
            set_selection(idx)
        end
    end)

    -- Remove a puzzle
    buttons.remove:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 0 then
            local id = puzzles._order[idx]
            -- Only remove puzzles that we created
            if is_user_puzzle(id) then
                puzzles:remove(id)
                puzzle_list:Delete(idx - 1)
                if idx == 1 then
                    set_selection(idx - 1)
                else
                    set_selection(idx - 2)
                end
            end
        end
    end)

    -- Add a puzzle
    buttons.add:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local puzzle = show_advanced_options_dialog(panel, "New Source")
        if puzzle then
            puzzles:insert(puzzle)
            local selection = puzzle_list.Selection
            update_sources()
            set_selection(selection)
        end
    end)


    -- Puzzle details
    -- ----------------------
    local detailsizer = panel.sources.detailsizer
    local details

    -- The text to show when no puzzle is selected
    local text = wx.wxStaticText(panel.sources, wx.wxID_ANY, "Select a Puzzle",
                                 wx.wxDefaultPosition, wx.wxDefaultSize,
                                 wx.wxALIGN_CENTER)
    detailsizer:Add(text, 1, wx.wxALIGN_CENTER)

    local function get_selected_puzzle()
        return puzzles:get(puzzle_list.Selection + 1)
    end

    local update_details

    local function on_select()
        update_details()
        -- Disable/enable the remove button
        buttons.remove:Enable(is_user_puzzle(get_selected_puzzle()))
    end

    set_selection = function(idx)
        puzzle_list.Selection = idx
        on_select()
    end


    local function do_advanced_options(puzzle)
        show_advanced_options_dialog(panel, puzzle)
        update_sources()
        update_details()
    end

    -- Events

    -- Update details panel on selection
    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_SELECTED, function(evt)
        on_select()
        evt:Skip()
    end)

    -- Show advanced options on double click
    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, function(evt)
        if puzzle_list.Selection ~= -1 then
            do_advanced_options(get_selected_puzzle())
            -- Disable/enable the remove button
            buttons.remove:Enable(is_user_puzzle(get_selected_puzzle()))
        end
        evt:Skip()
    end)

    -- Update the details panel based on the selected puzzle
    update_details = function()
        panel.sources:Freeze()
        if details then
            details:apply()
            detailsizer:Detach(details)
            details:Destroy()
        end
        if puzzle_list.Selection == -1 then
            detailsizer:Show(text, true)
            text.Label = "Select a Puzzle"
        else
            detailsizer:Show(text, false)
            details = get_download_options_panel(panel.sources, get_selected_puzzle())
            detailsizer:Add(details, 1, wx.wxEXPAND)
            detailsizer:Show(details, true)
            -- Connect the advanced options button
            details.advanced_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
                do_advanced_options(details.puzzle)
            end)

        end
        detailsizer:Layout()
        panel.sources:Thaw()
    end

    -- Load and save config
    -- --------------------------

    -- Load config values into the panel
    function panel:load_config()
        -- Basic info
        panel.puzzle_directory.Path = download.puzzle_directory
        panel.separate_directories.Selection = download.separate_directories and 1 or 0
        panel.auto_download.Value = download.auto_download or 0
        panel.default_view:SetStringSelection(stringx.capitalize(download.default_view))
        -- Text styles
        for name, _ in pairs(download.styles) do
            local style = panel.text_styles[name]
            style.font.SelectedFont = download.styles[name].font
            style.color.Colour = download.styles[name].color.AsString
            style.text.Font = download.styles[name].font
            style.text.ForegroundColour = download.styles[name].color
        end
        -- Puzzle sources
        puzzles = tablex.deepcopy(download.puzzles)
        update_sources()
    end

    -- Save config values from the panel
    function panel:apply()
        -- Basic info
        download.puzzle_directory = panel.puzzle_directory.Path
        download.separate_directories = panel.separate_directories.Selection == 1
        download.auto_download = panel.auto_download.Value
        download.default_view = panel.default_view.StringSelection:lower()
        -- Text styles
        for name, _ in pairs(download.styles) do
            local style = panel.text_styles[name]
            download.styles[name].font = style.font.SelectedFont
            download.styles[name].color = style.color.Colour
        end
        -- Puzzle sources
        if details then details:apply() end
        download.puzzles = tablex.deepcopy(puzzles)
        -- If we have an open downloader dialog make sure to update it
        if download.dialog then
            download.dialog:update()
        end

    end

    panel:load_config()
    return panel
end

local get_download_fields

-- Get a config panel for the selected puzzle
get_download_options_panel = function(parent, puzzle)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    -- wxLua isn't updated for 2.9, which includes wxST_ELLIPSIZE_MIDDLE = 8
    local name = wx.wxStaticText(panel, wx.wxID_ANY, puzzle.name,
                                 wx.wxDefaultPosition, wx.wxDefaultSize,
                                 wx.wxST_NO_AUTORESIZE + wx.wxALIGN_CENTER + 8)
    local font = name.Font
    font.PointSize = font.PointSize + 1
    name.Font = font
    sizer:Add(name, 0, wx.wxALIGN_CENTER + wx.wxALL, 5)

    local options = get_download_fields(panel, puzzle)
    sizer:Add(options, 1, wx.wxEXPAND)

    local advanced = wx.wxButton(panel, wx.wxID_ANY, "Advanced Options")
    sizer:Add(advanced, 0, wx.wxTOP + wx.wxALIGN_CENTER,  5)

    -- Public stuff
    function panel:apply()
        options:apply()
    end
    panel.advanced_button = advanced
    panel.puzzle = puzzle

    return panel
end


get_download_fields = function(parent, puzzle)
    -- If a source has options, add them to a 'fields' table (see NYT Premium
    -- for example)
    local function field_key(name)
        return name:gsub('%s', '_'):lower()
    end

    local ctrls = {}
    for _, name in ipairs(puzzle.fields or {}) do
        table.insert(ctrls, name)
        ctrls[name] = true
    end

    -- The main panel
    local scroller = ScrolledPanel(parent)

    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    scroller.panel:SetSizer(border)

    -- All sources get an "enabled" checkbox
    local enabled = wx.wxCheckBox(scroller.panel, wx.wxID_ANY, "Enabled")
    enabled.Value = not puzzle.disabled
    border:Add(enabled, 0, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT + wx.wxTOP, 5)

    -- The sizer for user-defined fields
    local sizer = wx.wxFlexGridSizer(0, 2, 5,5)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:AddGrowableCol(1)

    for _, name in ipairs(ctrls) do
        sizer:Add(wx.wxStaticText(scroller.panel, wx.wxID_ANY, name), 0, wx.wxALIGN_CENTER_VERTICAL)
        -- Make fields named "password" into password text boxes
        local flags = 0
        if name:lower():match("password") then
            flags = wx.wxTE_PASSWORD
        end
        -- Create the text ctrl
        local ctrl = wx.wxTextCtrl(
            scroller.panel, wx.wxID_ANY, puzzle[field_key(name)] or '',
            wx.wxDefaultPosition, wx.wxDefaultSize, flags)
        sizer:Add(ctrl, 1, wx.wxEXPAND)
        ctrls[name] = ctrl
    end

    function scroller:apply()
        for _, name in ipairs(ctrls) do
            local ctrl = ctrls[name]
            puzzle[field_key(name)] = ctrl.Value
        end
        puzzle.disabled = not enabled.Value
    end

    return scroller
end


xword.AddPreferencesPanel(
    "Downloader",
    download.get_config_panel,
    function (panel)
        panel:apply()
        download.save_config()
    end)

function download.show_config_dialog(parent)
    xword.showerrors = false
    local dlg = wx.wxDialog(parent or xword.frame, wx.wxID_ANY, "Download Config",
                            wx.wxDefaultPosition, wx.wxSize(500,500), wx.wxDEFAULT_FRAME_STYLE)

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    local panel = download.get_config_panel(dlg)
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)
    local buttons = dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL)
    sizer:Add(buttons, 0, wx.wxALL, 5)

    dlg:Fit()

    dlg:Center()
    if dlg:ShowModal() == wx.wxID_OK then
        panel:apply()
        download.save_config()
    end
    dlg:Destroy()
end