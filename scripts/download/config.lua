local join = require 'pl.path'.join
local tablex = require 'pl.tablex'
local stringx = require 'pl.stringx'
require 'safe_exec'
require 'os'
require 'lfs'

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
download.disabled = {
    "NY Times Premium",
    "NY Times (XWord Info)",
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
    missing = { font = wx.wxFont(wx.wxSWISS_FONT), color = wx.wxBLUE },
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

-- Disabled
local dl_list = config.disabled or download.disabled
download.disabled = {}
for _, id in ipairs(dl_list) do
    download.disabled[id] = true
end

-- Add download sources
for _, puzzle in ipairs(config.added or {}) do
    download.puzzles:insert(puzzle)
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
    for id, disabled in pairs(download.disabled or {}) do
        if disabled then
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
                        else
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

require 'luacurl'
local ArrowButton = require 'download.arrow_button'
local TextButton = require 'download.text_button'
local BmpButton = require 'download.bmp_button'
local bmp = require 'download.bmp'


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

local function get_download_fields(parent, puzzle)
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

    local scroller = scrolled_panel(parent)

    local border = wx.wxBoxSizer(wx.wxVERTICAL)
    scroller.panel:SetSizer(border)

    local enabled = wx.wxCheckBox(scroller.panel, wx.wxID_ANY, "Enabled")
    enabled.Value = not download.disabled[puzzle.id]
    border:Add(enabled, 0, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT + wx.wxTOP, 5)

    local sizer = wx.wxFlexGridSizer(0, 2, 5,5)
    border:Add(sizer, 1, wx.wxEXPAND + wx.wxALL, 5)
    sizer:AddGrowableCol(1)

    for _, name in ipairs(ctrls) do
        sizer:Add(wx.wxStaticText(scroller.panel, wx.wxID_ANY, name), 0, wx.wxALIGN_CENTER_VERTICAL)
        local flags = 0
        if name:lower():match("password") then
            flags = wx.wxTE_PASSWORD
        end
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
        download.disabled[puzzle.id] = not enabled.Value
    end

    return scroller
end

local function get_download_options_panel(parent, puzzle)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local options = get_download_fields(panel, puzzle)
    sizer:Add(options, 1, wx.wxEXPAND)

    local advanced = wx.wxButton(panel, wx.wxID_ANY, "Advanced Options")
    sizer:Add(advanced, 0, wx.wxTOP + wx.wxALIGN_CENTER,  5)

    advanced:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        show_advanced_options_dialog(panel, puzzle)
        parent.list:update_sources()
        parent:update_details()
    end)

    function panel:apply()
        options:apply()
    end
    return panel
end


function download.get_config_panel(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxGridBagSizer(5,5)
    sizer:AddGrowableCol(1)
    panel:SetSizer(sizer)
    local function sizerAdd(obj, pos, span, flags, border)
        return sizer:Add(obj,
            wx.wxGBPosition(unpack(pos)), wx.wxGBSpan(unpack(span or {1,1})),
            flags or 0, border or 0)
    end

    local sizer1 = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizerAdd(sizer1, {0,0}, {1,2}, wx.wxEXPAND)

    local puzzle_directory = wx.wxDirPickerCtrl(panel, wx.wxID_ANY, download.puzzle_directory)
    sizer1:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Download Directory:"), 0, wx.wxALIGN_CENTER)
    sizer1:Add(puzzle_directory, 1, wx.wxEXPAND)

    local separate_directories = wx.wxRadioBox(
        panel, wx.wxID_ANY, "Download puzzles to",
        wx.wxDefaultPosition, wx.wxDefaultSize,
        {"One directory", "Directories by source"}, 2
    )
    separate_directories.Selection = download.separate_directories and 1 or 0
    sizerAdd(separate_directories, {1,0}, {1,1}, wx.wxEXPAND)

    -- Auto download
    local autosizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Automatically download")
    sizerAdd(autosizer, {2,0}, {1,1}, wx.wxEXPAND)
    autosizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Last"), 0, wx.wxALIGN_CENTER_VERTICAL)
    local auto_download = wx.wxSpinCtrl(
        panel, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxSize(100, -1),
        wx.wxSP_ARROW_KEYS, 0, 30, download.auto_download or 0)
    autosizer:Add(auto_download, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxLEFT + wx.wxRIGHT, 5)
    autosizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "day(s) [0 = disabled]"), 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Default view
    local viewsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizerAdd(viewsizer, {3,0}, {1,1}, wx.wxEXPAND + wx.wxLEFT + wx.wxRIGHT, 5)
    viewsizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Default dialog view:"),
                  0, wx.wxALIGN_CENTER_VERTICAL)
    viewsizer:AddStretchSpacer()
    local default_view = wx.wxChoice(panel, wx.wxID_ANY, wx.wxDefaultPosition,
        wx.wxDefaultSize, {"Day", "Week", "Month", "Previous view"})
    default_view:SetStringSelection(stringx.capitalize(download.default_view))
    viewsizer:Add(default_view, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Text styles
    local stylesizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Text styles")
    sizerAdd(stylesizer, {1,1}, {3,1}, wx.wxEXPAND)

    local stylegrid = wx.wxGridSizer(0, 3, 5, 5)
    stylesizer:Add(stylegrid, 1, wx.wxALL, 5)

    local function make_style(label, style)
        local text = wx.wxStaticText(panel, wx.wxID_ANY, label)
        text.Font = style.font
        text.ForegroundColour = style.color
        local font = wx.wxFontPickerCtrl(panel, wx.wxID_ANY, style.font, wx.wxDefaultPosition, wx.wxDefaultSize, 0)
        font:Connect(wx.wxEVT_COMMAND_FONTPICKER_CHANGED, function (evt)
            style.font = evt.Font
            text.Font = evt.Font
        end)
        local color = wx.wxColourPickerCtrl(panel, wx.wxID_ANY, style.color)
        color:Connect(wx.wxEVT_COMMAND_COLOURPICKER_CHANGED, function (evt)
            style.color = evt.Colour
            text.Colour = evt.Colour
        end)
        stylegrid:Add(text)
        stylegrid:Add(font)
        stylegrid:Add(color)
    end

    make_style("Missing", download.styles.missing)
    make_style("Downloaded", download.styles.downloaded)
    make_style("In Progress", download.styles.progress)
    make_style("Complete", download.styles.complete)

    -- Puzzle Sources
    local srcsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)

    local listsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Puzzle sources")
    sizerAdd(listsizer, {4,0}, {1,1}, wx.wxEXPAND)

    local puzzle_list = wx.wxListBox(panel, wx.wxID_ANY, wx.wxDefaultPosition,
                                     wx.wxSize(-1, 150))
    panel.list = puzzle_list
    listsizer:Add(puzzle_list, 1, wx.wxEXPAND)
    
    local puzzles = tablex.deepcopy(download.puzzles)
    function puzzle_list:update_sources()
        local names = {}
        for key, puzzle in puzzles:iterall() do
            table.insert(names, puzzle.name)
        end
        local selection = puzzle_list.Selection
        puzzle_list:Set(names)
        puzzle_list.Selection = selection
    end

    -- List buttons
    local buttonsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    listsizer:Add(buttonsizer, 0, wx.wxALL + wx.wxALIGN_RIGHT, 5)

    local btn_up = BmpButton(panel, wx.wxID_ANY, bmp.up)
    buttonsizer:Add(btn_up, 0, wx.wxLEFT, 5)
    btn_up.ToolTip = wx.wxToolTip("Move up in list")
    btn_up:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 1 then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx-1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(puzzles[id].name, idx - 2)
            puzzle_list.Selection = idx - 2
        end
    end)

    local btn_down = BmpButton(panel, wx.wxID_ANY, bmp.down)
    buttonsizer:Add(btn_down, 0, wx.wxLEFT, 5)
    btn_down.ToolTip = wx.wxToolTip("Move down in list")
    btn_down:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx < #puzzles._order then
            local id = table.remove(puzzles._order, idx)
            table.insert(puzzles._order, idx+1, id)
            puzzle_list:Delete(idx - 1)
            puzzle_list:Insert(puzzles[id].name, idx)
            puzzle_list.Selection = idx
        end
    end)

    local btn_remove = BmpButton(panel, wx.wxID_ANY, bmp.remove)
    buttonsizer:Add(btn_remove, 0, wx.wxLEFT, 5)
    btn_remove.ToolTip = wx.wxToolTip("Remove from list")
    btn_remove:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local idx = puzzle_list.Selection + 1
        if idx > 0 then
            local id = puzzles._order[idx]
            -- Only remove puzzles that we created
            if download.get_default_puzzles()[id] == nil then
                puzzles:remove(id)
                puzzle_list:Delete(idx - 1)
                if idx == 1 then
                    puzzle_list.Selection = idx - 1
                else
                    puzzle_list.Selection = idx - 2
                end
            end
        end
    end)


    local btn_add = BmpButton(panel, wx.wxID_ANY, bmp.add)
    buttonsizer:Add(btn_add, 0, wx.wxLEFT, 5)
    btn_add.ToolTip = wx.wxToolTip("Add a new source")
    btn_add:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        local puzzle = show_advanced_options_dialog(panel, "New Source")
        if puzzle then
            puzzles:insert(puzzle)
            local selection = puzzle_list.Selection
            puzzle_list:update_sources()
            puzzle_list.Selection = selection
        end
    end)

    -- Details panel
    local detailsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Configuration")
    sizerAdd(detailsizer, {4,1}, {1,1}, wx.wxEXPAND)
    local details

    local text = wx.wxStaticText(panel, wx.wxID_ANY, "Select a Puzzle",
                                 wx.wxDefaultPosition, wx.wxDefaultSize,
                                 wx.wxALIGN_CENTER)
    detailsizer:Add(text, 1, wx.wxALIGN_CENTER)

    function panel:update_details()
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
            details = get_download_options_panel(panel, puzzles:get(puzzle_list.Selection + 1))
            detailsizer:Add(details, 1, wx.wxEXPAND)
            detailsizer:Show(details, true)
        end
        detailsizer:Layout()
    end

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_SELECTED, function(evt)
        panel:update_details()
        evt:Skip()
    end)

    puzzle_list:Connect(wx.wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, function(evt)
        evt:Skip()
        if puzzle_list.Selection ~= -1 then
            show_advanced_options_dialog(panel, puzzles:get(puzzle_list.Selection + 1))
            puzzle_list:update_sources()
            panel:update_details()
        end
    end)

    panel.MinSize = wx.wxSize(500, -1)

    function panel:apply()
        download.puzzle_directory = puzzle_directory.Path
        download.separate_directories = separate_directories.Selection == 1
        download.default_view = default_view:GetStringSelection():lower()
        if details then details:apply() end
        download.puzzles = tablex.deepcopy(puzzles)
        download.auto_download = auto_download.Value
        if download.dialog then
            download.dialog:update()
        end
    end
    
    puzzle_list:update_sources()

    return panel
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