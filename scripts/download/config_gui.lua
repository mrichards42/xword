-- ============================================================================
-- Widgets
-- ============================================================================

require 'luacurl'
local ArrowButton = require 'download.arrow_button'
local TextButton = require 'download.text_button'
local BmpButton = require 'download.bmp_button'
local bmp = require 'download.bmp'
local tablex = require 'pl.tablex'

-- A basic scrolled panel
function scrolled_panel(parent)
    local scroller = wx.wxScrolledWindow(parent, wx.wxID_ANY)
    scroller:SetWindowStyle(wx.wxBORDER_DOUBLE)
    scroller:SetScrollRate(10, 10)

    function scroller:update_scrollbars()
        self.panel:Layout()
        self:FitInside()
        self:Refresh()
    end

    -- This panel preserves tab behavior
    local psizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    local panel = wx.wxPanel(scroller, wx.wxID_ANY)
    psizer:Add(panel, 1, wx.wxEXPAND)
    scroller:SetSizer(psizer)

    scroller.panel = panel
    return scroller
end


-- A panel with a title and a collapse button
function collapse_panel(parent, label)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    -- Header
    local header = wx.wxBoxSizer(wx.wxHORIZONTAL)
    sizer:Add(header, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    local arrow = ArrowButton(panel, wx.wxID_ANY)
    header:Add(arrow, 0, wx.wxALIGN_CENTER_VERTICAL + wx.wxRIGHT, 5)
    local text = TextButton(panel, wx.wxID_ANY, label)
    header:Add(text, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Collapsing
    local inside_panel
    local function update_panel()
        if inside_panel then
            sizer:Show(inside_panel, arrow:IsExpanded())
            sizer:Layout()
            parent.Parent:Fit()
        end
    end

    arrow:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, update_panel)
    
    text:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        arrow:Toggle()
        update_panel()
    end)

    -- Public stuff
    panel.header = header
    panel.sizer = sizer
    function panel:set_panel(p)
        inside_panel = p
        sizer:Add(inside_panel, 1, wx.wxEXPAND)
        update_panel()
    end

    function panel:set_label(label)
        text:SetLabel(label)
    end

    return panel
end


-- ============================================================================
-- Advanced Options Dialog
-- ============================================================================

local advanced_options_panel

-- The dialog
function advanced_options_dialog(parent, title)
    local dlg = wx.wxDialog(parent, wx.wxID_ANY, title or "Advanced Options")

    -- Layout
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    dlg:SetSizer(sizer)
    local panel = advanced_options_panel(dlg)
    panel.MinSize = wx.wxSize(400, -1)
    sizer:Add(panel, 1, wx.wxEXPAND + wx.wxALL, 5)

    local buttons = dlg:CreateButtonSizer(wx.wxOK + wx.wxCANCEL)
    sizer:Add(buttons, 0, wx.wxALL, 5)

    local reset_button = wx.wxButton(dlg, wx.wxID_ANY, "Reset to Defaults")
    buttons:Add(reset_button)

    dlg:Fit()
    dlg.MinSize = dlg.Size
    dlg:Center()

    -- Public stuff
    dlg.panel = panel
    dlg.reset_button = reset_button

    return dlg
end


local curl_options_panel
local download_fields_panel
local download_function_panel

-- The inside panel
advanced_options_panel = function(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
    panel:SetSizer(sizer)

    local grid = wx.wxFlexGridSizer(0, 2, 5, 5)
    grid:AddGrowableCol(1)
    sizer:Add(grid, 0, wx.wxEXPAND + wx.wxBOTTOM, 5)

    -- Basic info
    local name = wx.wxTextCtrl(panel, wx.wxID_ANY)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Name:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(name, 1, wx.wxEXPAND)

    local url = wx.wxTextCtrl(panel, wx.wxID_ANY)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "URL:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(url, 1, wx.wxEXPAND)

    local filename = wx.wxTextCtrl(panel, wx.wxID_ANY)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Local filename:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(filename, 1, wx.wxEXPAND)

    local directoryname = wx.wxTextCtrl(panel, wx.wxID_ANY)
    grid:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Subdirectory:"), 0, wx.wxALIGN_CENTER_VERTICAL)
    grid:Add(directoryname, 1, wx.wxEXPAND)

    -- Days checkboxes
    local daysizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Days Available")
    sizer:Add(daysizer, 0)
    local days = {}
    local daygrid = wx.wxGridSizer(1, 0, 5, 5)
    daysizer:Add(daygrid, 1, wx.wxEXPAND + wx.wxALL, 5)
    for i, label in ipairs({'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'}) do
        local ctrl = wx.wxCheckBox(panel, wx.wxID_ANY, label)
        table.insert(days, ctrl)
        daygrid:Add(ctrl, 1, wx.wxEXPAND)
    end

    -- cURL options
    local curlopts = curl_options_panel(panel)
    sizer:Add(curlopts, 0, wx.wxEXPAND + wx.wxTOP, 10)

    -- fields
    local fields = download_fields_panel(panel)
    sizer:Add(fields, 0, wx.wxEXPAND + wx.wxTOP, 10)

    -- custom function
    local custom_function = download_function_panel(panel)
    sizer:Add(custom_function, 0, wx.wxEXPAND + wx.wxTOP, 10)

    -- Public stuff
    panel.name = name
    panel.url = url
    panel.filename = filename
    panel.directoryname = directoryname
    panel.days = days
    panel.curlopts = curlopts
    panel.fields = fields
    panel.custom_function = custom_function.text

    return panel
end


-- cURL options
curl_options_panel = function(parent)
    local panel = collapse_panel(parent, "Download Options")

    -- Add a help button to the header
    panel.header:AddStretchSpacer()
    local link = TextButton(panel, wx.wxID_ANY, "(help)")
    link:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
        wx.wxLaunchDefaultBrowser(
            "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html")
    end)
    link.ForegroundColour = wx.wxBLUE
    panel.header:Add(link)

    -- The main panel
    local scroller = scrolled_panel(panel, wx.wxID_ANY)
    panel:set_panel(scroller)

    local grid = wx.wxFlexGridSizer(0, 3, 5, 5)
    grid:AddGrowableCol(1)
    scroller.panel:SetSizer(grid)

    -- Make a list of curl options
    local curl_opt_list = {}
    local curl_opt_names = {}
    for k, v in pairs(curl) do
        if k:sub(1,4) == "OPT_" then
            table.insert(curl_opt_list, k:sub(5))
            curl_opt_names[v] = k:sub(5)
        end
    end
    table.sort(curl_opt_list)

    local opts = {}

    -- The add button
    grid:AddSpacer(0)
    grid:AddSpacer(0)
    local add_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.add)
    grid:Add(add_button, 0, wx.wxALIGN_CENTER_VERTICAL)
    add_button.ToolTip = wx.wxToolTip("Add option")
    add_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        scroller:Freeze()
        panel:add_option()
        scroller:Thaw()
    end)

    scroller.MinSize = wx.wxSize(-1, 100)

    -- Public stuff
    panel.opts = opts

    -- Add the GUI elements for a new option
    function panel:add_option(id, value)
        if id then -- Make sure this is a valid option
            if not curl_opt_names[id] then
                return false
            end
        end
        local opt = wx.wxChoice(
            scroller.panel, wx.wxID_ANY,
            wx.wxDefaultPosition, wx.wxDefaultSize, curl_opt_list)
        opt.StringSelection = curl_opt_names[id] or ''
        local text = wx.wxTextCtrl(scroller.panel, wx.wxID_ANY, value or '')
        local remove_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.remove)
        remove_button.ToolTip = wx.wxToolTip("Remove option")
        grid:Insert(#opts * 3, opt)
        grid:Insert(#opts * 3 + 1, text, 1, wx.wxEXPAND)
        grid:Insert(#opts * 3 + 2, remove_button, 0, wx.wxALIGN_CENTER_VERTICAL)
        table.insert(opts, {opt, text})
        scroller:update_scrollbars()

        -- Remove event
        remove_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
            for i, ctrls in ipairs(opts) do
                if ctrls[1] == opt then
                    table.remove(opts, i)
                    grid:Detach((i-1) * 3)
                    grid:Detach((i-1) * 3)
                    grid:Detach((i-1) * 3)
                    opt:Destroy()
                    text:Destroy()
                    remove_button:Destroy()
                    scroller:update_scrollbars()
                    break
                end
            end
        end)
    end

    function panel:clear()
        -- Clear the grid sizer
        for i=0,#opts*3-1 do
            local window = grid:GetItem(0).Window
            grid:Detach(0)
            window:Destroy()
        end
        tablex.clear(opts)
        scroller:update_scrollbars()
    end

    return panel
end


-- Custom fields
download_fields_panel = function(parent)
    local panel = collapse_panel(parent, "Custom User Options")

    -- Fields
    local scroller = scrolled_panel(panel, wx.wxID_ANY)
    panel:set_panel(scroller)

    local grid = wx.wxFlexGridSizer(0, 2, 5, 5)
    grid:AddGrowableCol(0)
    scroller.panel:SetSizer(grid)

    local fields = {}

    -- The add button
    grid:AddSpacer(0)
    local add_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.add)
    grid:Add(add_button, 0, wx.wxALIGN_CENTER_VERTICAL)
    add_button.ToolTip = wx.wxToolTip("Add field")
    add_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function (evt)
        scroller:Freeze()
        panel:add_field()
        scroller:Thaw()
    end)

    scroller.MinSize = wx.wxSize(-1, 100)

    -- Public stuff
    panel.fields = fields

    function panel:add_field(value)
        local text = wx.wxTextCtrl(scroller.panel, wx.wxID_ANY, value or '')
        local remove_button = BmpButton(scroller.panel, wx.wxID_ANY, bmp.remove)
        remove_button.ToolTip = wx.wxToolTip("Remove field")
        grid:Insert(#fields * 2, text, 1, wx.wxEXPAND)
        grid:Insert(#fields * 2 + 1, remove_button, 0, wx.wxALIGN_CENTER_VERTICAL)
        table.insert(fields, text)
        scroller:update_scrollbars()

        -- Remove event
        remove_button:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED, function(evt)
            for i, ctrl in ipairs(fields) do
                if ctrl == text then
                    table.remove(fields, i)
                    grid:Detach((i-1) * 2)
                    grid:Detach((i-1) * 2)
                    text:Destroy()
                    remove_button:Destroy()
                    scroller:update_scrollbars()
                    break
                end
            end
        end)
    end

    function panel:clear()
        -- Clear the grid sizer
        for i=0,#fields*2-1 do
            local window = grid:GetItem(0).Window
            grid:Detach(0)
            window:Destroy()
        end
        tablex.clear(fields)
        scroller:update_scrollbars()
    end

    return panel
end


-- Custom download function
download_function_panel = function(parent)
    local panel = collapse_panel(parent, "Custom Download Function")

    local font = wx.wxFont(8, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_NORMAL,
                           wx.wxFONTWEIGHT_NORMAL, false)

    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

    local st = wx.wxStaticText(panel, wx.wxID_ANY, "function(puzzle)")
    st.Font = font
    sizer:Add(st, 0, wx.wxLEFT, 10)

    local text = wx.wxTextCtrl(panel, wx.wxID_ANY, '',
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTE_MULTILINE + wx.wxHSCROLL)
    text.Font = font
    text.MinSize = wx.wxSize(-1, 100)
    sizer:Add(text, 1, wx.wxEXPAND + wx.wxLEFT, 20)

    local st = wx.wxStaticText(panel, wx.wxID_ANY, "end")
    st.Font = font
    sizer:Add(st, 0, wx.wxLEFT, 10)

    panel:set_panel(sizer)

    -- Public stuff
    panel.text = text

    return panel
end


-- ============================================================================
-- Config Panel
-- ============================================================================

-- The panel
local text_styles_panel
local puzzle_sources_panel

function config_panel(parent)
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

    -- Download directory
    local puzzle_directory = wx.wxDirPickerCtrl(panel, wx.wxID_ANY, "")
    sizer1:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Download Directory:"), 0, wx.wxALIGN_CENTER)
    sizer1:Add(puzzle_directory, 1, wx.wxEXPAND)

    -- Separate directories radio box
    local separate_directories = wx.wxRadioBox(
        panel, wx.wxID_ANY, "Download puzzles to",
        wx.wxDefaultPosition, wx.wxDefaultSize,
        {"One directory", "Directories by source"}, 2
    )
    separate_directories.Selection = 1
    sizerAdd(separate_directories, {1,0}, {1,1}, wx.wxEXPAND)

    -- Auto download
    local autosizer = wx.wxStaticBoxSizer(wx.wxHORIZONTAL, panel, "Automatically download")
    sizerAdd(autosizer, {2,0}, {1,1}, wx.wxEXPAND)
    autosizer:Add(wx.wxStaticText(panel, wx.wxID_ANY, "Last"), 0, wx.wxALIGN_CENTER_VERTICAL)
    local auto_download = wx.wxSpinCtrl(
        panel, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxSize(100, -1),
        wx.wxSP_ARROW_KEYS, 0, 30, 0)
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
    default_view:SetStringSelection("Week")
    viewsizer:Add(default_view, 0, wx.wxALIGN_CENTER_VERTICAL)

    -- Text styles
    local text_styles = text_styles_panel(panel)
    local style_sizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Text styles")
    style_sizer:Add(text_styles, 1, wx.wxALL, 5)
    sizerAdd(style_sizer, {1, 1}, {3, 1}, wx.wxEXPAND)

    -- Puzzle Sources
    local sources = puzzle_sources_panel(panel)
    sizerAdd(sources, {4, 0}, {1, 2}, wx.wxEXPAND)

    panel.MinSize = wx.wxSize(500, -1)

    -- Public stuff
    panel.puzzle_directory = puzzle_directory
    panel.separate_directories = separate_directories
    panel.auto_download = auto_download
    panel.default_view = default_view
    panel.text_styles = text_styles.styles
    panel.sources = sources

    return panel
end


-- Text styles
text_styles_panel = function(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)
    local sizer = wx.wxGridSizer(0, 3, 5, 5)
    panel:SetSizer(sizer)

    -- A style selector with font and color
    local function make_style(label)
        local text = wx.wxStaticText(panel, wx.wxID_ANY, label)
        local font = wx.wxFontPickerCtrl(panel, wx.wxID_ANY)
        local color = wx.wxColourPickerCtrl(panel, wx.wxID_ANY, wx.wxNullColour)
        -- Update the label when the style changes
        color:Connect(wx.wxEVT_COMMAND_COLOURPICKER_CHANGED, function (evt)
            text.ForegroundColour = evt.Colour
            text:Refresh()
        end)
        font:Connect(wx.wxEVT_COMMAND_FONTPICKER_CHANGED, function (evt)
            text.Font = evt.Font
            text:Refresh()
        end)
        -- Add the items to our grid
        sizer:Add(text)
        sizer:Add(font)
        sizer:Add(color)
        return { font = font, color = color, text = text }
    end

    panel.styles = {
        missing = make_style("Missing"),
        downloaded = make_style("Downloaded"),
        progress = make_style("In Progress"),
        complete = make_style("Complete")
    }

    return panel
end


-- Puzzle sources
puzzle_sources_panel = function(parent)
    local panel = wx.wxPanel(parent, wx.wxID_ANY)

    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    panel:SetSizer(sizer)

    local listsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Puzzle sources")
    sizer:Add(listsizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    local puzzle_list = wx.wxListBox(panel, wx.wxID_ANY, wx.wxDefaultPosition,
                                     wx.wxSize(-1, 150))
    listsizer:Add(puzzle_list, 1, wx.wxEXPAND)

    -- List buttons
    local buttonsizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
    listsizer:Add(buttonsizer, 0, wx.wxALL + wx.wxALIGN_RIGHT, 5)

    local function make_button(tooltip, bitmap)
        local button = BmpButton(panel, wx.wxID_ANY, bitmap)
        buttonsizer:Add(button, 0, wx.wxLEFT, 5)
        button.ToolTip = wx.wxToolTip(tooltip)
        return button
    end

    local buttons = {
        up = make_button("Move up in list", bmp.up),
        down = make_button("Move down in list", bmp.down),
        remove = make_button("Remove from list", bmp.remove),
        add = make_button("Add a new source", bmp.add)
    }

    -- Details panel
    local detailsizer = wx.wxStaticBoxSizer(wx.wxVERTICAL, panel, "Configuration")
    sizer:Add(detailsizer, 1, wx.wxEXPAND + wx.wxALL, 5)

    -- Public stuff
    panel.list = puzzle_list
    panel.detailsizer = detailsizer
    panel.buttons = buttons

    return panel
end
