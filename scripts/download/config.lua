local join = require 'pl.path'.join
local tablex = require 'pl.tablex'
local stringx = require 'pl.stringx'
require 'safe_exec'
require 'os'
require 'lfs'
require 'date'

require 'luacurl'
local curl_opt_names = {}
local curl_opt_table = {}
for k, v in pairs(curl) do
    if k:sub(1,4) == "OPT_" then
        k = k:sub(5)
        curl_opt_names[v] = k
        curl_opt_table[k] = v
    end
end

-- ============================================================================
-- Default config
-- ============================================================================
download.puzzle_directory = join(xword.userdatadir, "puzzles")
download.separate_directories = true
download.auto_download = 0
download.default_view = "Day"
download.previous_view = {}
default_disabled = {
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
download.styles = {
    missing = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(wx.wxBLUE) },
    downloaded = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(128, 0, 128) },
    progress = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(34, 139, 34) },
    complete = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxColour(34, 139, 34) },
}

download.styles.progress.font.Weight = wx.wxFONTWEIGHT_BOLD


-- ============================================================================
-- Load config
-- ============================================================================
local function get_config_filename()
    return join(xword.configdir, 'download', 'config.lua')
end

local config, err = safe_dofile(get_config_filename())
if not config and lfs.attributes(get_config_filename(), 'mode') == 'file' then
    local msg =
        "Error while loading Puzzle Downloader configuration file.\n" ..
        "Would you like to reset Puzzle Downloader settings permanently?"
    if type(err) == 'string' and #err > 0 then
        msg = msg .. "\n\nDetails:\n" .. err
    end
    local rc = wx.wxMessageBox(msg, "XWord Error", wx.wxYES_NO + wx.wxICON_ERROR)
    if rc == wx.wxYES then
        os.remove(get_config_filename())
    end
end

if type(config) ~= 'table' then
    config = {}
end

-- Basic options
local basic_options = {
    'puzzle_directory', 'separate_directories',
    'auto_download','default_view', 'previous_view'}

for _, name in ipairs(basic_options) do
    if config[name] ~= nil then
        download[name] = config[name]
    end
end

-- Previous view
if not config.previous_view then config.previous_view = {} end
if config.previous_view.start_date then
    config.previous_view.start_date = date(config.previous_view.start_date)
end
if config.previous_view.end_date then
    config.previous_view.end_date = date(config.previous_view.end_date)
end

-- Styles
if not config.styles then config.styles = {} end
for k, style in pairs(download.styles) do
    local config_style = config.styles[k]
    if config_style then
        if config_style.font then
            style.font:SetNativeFontInfo(config_style.font)
        end
        if config_style.color then
            style.color:Set(config_style.color)
        end
    end
end

-- Add download sources
for _, puzzle in ipairs(config.added or {}) do
    download.puzzles:insert(puzzle)
end

-- Disabled
local dl_list = config.disabled or default_disabled
for _, id in ipairs(dl_list) do
    local puzzle = download.puzzles[id]
    if puzzle then
        puzzle.disabled = true
    end
end

-- Changed sources
for key, data in pairs(config.changed or {}) do
    local function update(puz1, puz2)
        -- Update nil values
        for _, k in ipairs(puz2._nil or {}) do
            puz1[k] = nil
        end
        puz2._nil = nil
        -- Copy puz2 to puz1
        for k,v in pairs(puz2) do
            if type(v) == 'table' then
                if type(puz1[k]) ~= 'table' then
                    puz1[k] = {}
                end
                update(puz1[k], puz2[k])
            else
                puz1[k] = v
            end
        end
    end

    local puzzle = download.puzzles[key]
    if puzzle then
        -- Make string curl options into numbers
        if data.curlopts then
            local curlopts = tablex.deepcopy(data.curlopts)
            tablex.clear(data.curlopts)
            for id, value in pairs(curlopts) do
                if type(id) == 'string' then
                    data.curlopts[curl_opt_table[id]] = value
                else
                    data.curlopts[id] = value
                end
            end
        end
        update(puzzle, data)
    end
end

-- Adjust the order
local oldorder = download.puzzles._order
download.puzzles._order = {}
for _, id in ipairs(config.order or {}) do
    -- Look for this id in the order.
    for i, puzid in ipairs(oldorder) do
        if puzid == id then
            table.insert(download.puzzles._order, id)
            table.remove(oldorder, i)
        end
    end
end

-- Add the rest of the puzzles in order
for _, id in ipairs(oldorder) do
    table.insert(download.puzzles._order, id)
end


-- ============================================================================
-- Save config
-- ============================================================================
function download.save_config()
    local config = {
        puzzle_directory = download.puzzle_directory,
        separate_directories = download.separate_directories,
        auto_download = download.auto_download,
        default_view = download.default_view,
        previous_view = {
            kind = download.previous_view.kind,
        },
        disabled = {},
        added = {},
        changed = {},
        order = download.puzzles._order,
        styles = {}
    }
    -- Previous view
    if download.previous_view.start_date then
        config.previous_view.start_date = download.previous_view.start_date:fmt("%m/%d/%Y")
    end
    if download.previous_view.end_date then
        config.previous_view.end_date = download.previous_view.end_date:fmt("%m/%d/%Y")
    end
    -- disabled
    for id, puzzle in pairs(download.puzzles or {}) do
        if puzzle.disabled then
            table.insert(config.disabled, id)
        end
    end
    -- Styles
    for k, style in pairs(download.styles) do
        config.styles[k] = { font = style.font:GetNativeFontInfoDesc(),
                             color = style.color:GetAsString() }
    end
    -- Figure out what has changed from the defaults
    local defaults = download.get_default_puzzles()
    for _, puzzle in download.puzzles:iterall() do
        local default = defaults[puzzle.id]
        if default then
            local function get_changes(puz1, puz2)
                local changes = { _nil = {} }
                local changed = false
                -- Check for changed values
                for k,v in pairs(puz1) do
                    if puz2[k] ~= v then
                        if type(puz1[k]) == 'table' and type(puz2[k]) == 'table' then
                            changes[k] = get_changes(puz1[k], puz2[k])
                            changed = changed or changes[k]
                        elseif k ~= "disabled" then
                            changes[k] = v
                            changed = true
                        end
                    end
                end
                -- Check for removed / nil values
                for k,v in pairs(puz2) do
                    if puz1[k] == nil then
                        table.insert(changes._nil, k)
                    end
                end
                if #changes._nil == 0 then
                    changes._nil = nil
                else
                    changed = true
                end
                if changed then return changes end
            end
            config.changed[puzzle.id] = get_changes(puzzle, default)
            -- Make curl options into strings
            if config.changed[puzzle.id] then
                config.changed[puzzle.id] = tablex.deepcopy(config.changed[puzzle.id])
                local opts = config.changed[puzzle.id].curlopts
                if opts then
                    local curlopts = tablex.deepcopy(opts)
                    tablex.clear(opts)
                    for id, value in pairs(curlopts) do
                        opts[curl_opt_names[id]] = value
                    end
                end
            end
        else
            table.insert(config.added, puzzle)
        end
    end

    serialize.pdump(config, get_config_filename())
end

xword.OnCleanup(download.save_config)

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