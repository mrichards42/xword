local _R = mod_path(...)
local _RR = mod_path(..., 2)

local wxfb = require(_R .. 'wxFB')
local DragAndDropMixin = require 'wx.lib.mixins.listctrl'.DragAndDropMixin
local TextButton = require 'wx.lib.text_button'
local TextCombo = require 'wx.lib.textcombo'
local tablex = require 'pl.tablex'
local stringx = require 'pl.stringx'
local curl = require 'luacurl'

local function PuzzleSourceDialog(parent, source)
    local self = wxfb.PuzzleSourceDialog(parent)
    local config = require(_RR .. 'config')

    -- Setup curlopts
    -- --------------
    local curlopts = tablex.values(curl.opts)
    table.sort(curlopts)
    self.curl_opts:AddColumn("cURL Option", function(parent, id) return TextCombo(parent, id, "", wx.wxDefaultPosition, wx.wxDefaultSize, curlopts, wx.wxCB_READONLY) end, 'Value', 2)
    self.curl_opts:AddColumn("Value", wx.wxTextCtrl, nil, 3)
    self.curl_opts:SetAutoAdd(true)

    -- Custom function
    -- ---------------
    self.notebook:RemovePage(self.notebook:GetPageCount()-1) -- Hide custom function
    self.func:SetMargins(10, -1)
     -- Set cursor for the message/button
    local cursor = wx.wxCursor(wx.wxCURSOR_HAND)
    self.custom_func_message:SetCursor(cursor)
    cursor:delete()
    -- Hide message
    local custom_func_sizer = self.custom_func_message:GetContainingSizer()
    custom_func_sizer:Hide(self.custom_func_message)
    custom_func_sizer:Layout()

    -- Events
    -- ------
    local custom_directory = false
    function self:OnNameChanged(evt)
        if not custom_directory then
            self.directoryname:SetValue(evt:GetString())
        end
    end

    function self:OnDirectoryChanged(evt)
        custom_directory = self.name:GetValue() ~= evt:GetString() and
                            stringx.strip(evt:GetString()) ~= ''
    end

    function self:OnAuthRequired(evt)
        local enable = self.auth_required:GetValue()
        self.auth_url:Enable(enable)
        self.auth_user_id:Enable(enable)
        self.auth_password_id:Enable(enable)
    end

    function self:OnShowCustomFunction(evt)
        self.notebook:AddPage(self.custom_func_panel, "Custom Function", true)
    end

    function self:OnOK(evt)
        local success, err = self:SaveSource()
        if not success then
            xword.Message("Please correct the following error(s):\n" .. err)
        else
            evt:Skip()
        end
    end

    -- Load/Save
    -- ---------
    function self:LoadSource()
        -- Basic
        custom_directory = source.directoryname and
                            stringx.strip(source.directoryname) ~= '' and
                            source.directoryname ~= source.name
        self.name:SetValue(source.name or '')
        self.url:SetValue(source.url or '')
        self.filename:SetValue(source.filename or '')
        self.directoryname:SetValue(custom_directory and source.directoryname or source.name or '')
        for i=1,7 do
            self['day' .. i]:SetValue(source.days[i] or false)
        end
        -- Auth
        self.auth_required:SetValue(source.auth and true or false)
        if self.auth_required:GetValue() then
            self.auth_url:SetValue(source.auth.url or '')
            self.auth_user_id:SetValue(source.auth.user_id or '')
            self.auth_password_id:SetValue(source.auth.password_id or '')
        end
        self:OnAuthRequired() -- Enable/disable controls
        -- Advanced
        self.not_puzzle:SetValue(source.not_puzzle or false)
        -- curlopts
        for k, v in pairs(source.curlopts or {}) do
            local str = curl.opt_to_string(k)
            if str then
                self.curl_opts:Append(str, tostring(v))
            end
        end
        -- Custom function
        if source.func then
            self.func:SetValue(source.func)
            custom_func_sizer:Show(self.custom_func_message)
            custom_func_sizer:Layout()
        end
    end

    function self:SaveSource()
        -- Validate
        local auth
        if self.auth_required:GetValue() then
            auth = {
                url = stringx.strip(self.auth_url:GetValue()),
                user_id = stringx.strip(self.auth_user_id:GetValue()),
                password_id = stringx.strip(self.auth_password_id:GetValue()),
            }
            if auth.url == '' or auth.user_id == '' or auth.password_id == '' then
                return nil, "All authentication fields are required"
            end
        end
        -- Basic
        source.name = stringx.strip(self.name:GetValue())
        source.url = stringx.strip(self.url:GetValue())
        source.filename = stringx.strip(self.filename:GetValue())
        if custom_directory then
            source.directoryname = stringx.strip(self.directoryname:GetValue())
        else
            source.directoryname = nil
        end
        for i=1,7 do
            source.days[i] = self['day' .. i]:GetValue()
        end
        -- Auth
        source.auth = auth
        -- Advanced
        source.not_puzzle = self.not_puzzle:GetValue() and true or nil
        source.func = stringx.strip(self.func:GetValue())
        if source.func == '' then source.func = nil end
        -- curlopts
        local opts = self.curl_opts:GetData()
        if #opts > 0 then
            tablex.clear(source.curlopts or {})
            for _, data in pairs(opts) do
                local opt, value = curl.get_opt(data[1]), data[2]
                if opt then
                    if not source.curlopts then source.curlopts = {} end
                    source.curlopts[opt] = tonumber(value) or value
                end
            end
        else
            source.curlopts = nil
        end
        -- Custom function
        local func = self.func:GetValue()
        if stringx.strip(func) ~= '' then
            source.func = func
        elseif source.func then
            source.func = nil
        end
        return true
    end

    self:LoadSource()
    return self
end

local function EditSource(parent, source)
    local dlg = PuzzleSourceDialog(parent, source)
    --return dlg:ShowModal() == wx.wxID_OK
    dlg:Show()
end

-- config.PreferencesPanel
return function(parent, id)
    local self = wxfb.PreferencesPanel(parent, id)
    local sources = require(_RR .. 'sources')
    local config = require(_RR .. 'config')
    local sources_data = {} -- { list item data = source }

    -- Additional GUI config for self.source_list
    DragAndDropMixin(self.source_list)
    self.source_list:SetColumnName(1, 'Puzzle')

    -- Events
    -- ------
    function self:OnSourceSelected(evt)
        
    end

    function self:OnNewSource(evt)
        -- Create the new source
        local source = {
            name = "New Source",
            days = {true, true, true, true, true, true, true},
            url = "http://www.example.com",
            filename = "new%Y%m%d.puz"
        }
        if EditSource(self, source) then
            -- Insert in the list
            local idx = self.source_list:InsertItem(self.source_list:GetItemCount(), "New Source", true)
            local id = self.source_list:GetItemData(idx)
            self.source_list:Select(idx)
            self.source_list:EnsureVisible(idx)
            -- Add to actual source data
            sources_data[id] = source
        end
    end

    function self:OnEditSource(evt)
        local id = self.source_list:GetItemData(self.source_list:GetSelection())
        local source = sources_data[id]
        if source then
            EditSource(self, source)
        end
    end

    function self:OnRemoveSource(evt)
        local idx = self.source_list:GetFirstSelected()
        if idx > -1 then
            local id = self.source_list:GetItemData(idx)
            sources_data[id] = nil
            self.source_list:DeleteItem(idx)
        end
    end




    -- Save/Load
    -- --------

    function self:LoadConfig()
        -- Load config
        self.separate_directories:SetValue(config.separate_directories)
        self.auto_download:SetValue(config.auto_download)
        self.puzzle_directory:SetPath(config.puzzle_directory)
        -- Load puzzle sources
        local i = 0
        for _, source in sources:iterall() do
            -- Insert
            local idx = self.source_list:InsertItem(i, source.name, not source.disabled)
            i = i + 1
            -- Map item data to the source
            sources_data[self.source_list:GetItemData(idx)] = source
        end
    end

    function self:SaveConfig()
        -- Save config
        config.separate_directories = self.separate_directories:GetValue()
        config.auto_download = self.auto_download:GetValue()
        config.puzzle_directory = self.puzzle_directory:GetPath()
        -- Save sources
        local new_sources = {}
        for i = 0, self.source_list:GetItemCount() - 1 do
            local id = self.source_list:GetItemData(i)
            local source = assert(sources_data[id])
            source.disabled = not self.source_list:IsChecked(i)
            table.insert(new_sources, source)
        end
        sources:set(new_sources)
    end

    self:LoadConfig()
    return self
end