-- Converted from python by py2lua
-- python file: wxFB.py
-- modtime: 1398906624
-- ----------------------------------

local wxfb = {} -- Table to hold classes

---------------------------------------------------------------------------
-- Python code generated with wxFormBuilder (version Nov  6 2013)
-- http://www.wxformbuilder.org/
--
-- PLEASE DO "NOT" EDIT THIS FILE!
---------------------------------------------------------------------------

---------------------------------------------------------------------------
-- Class DebugDialog
---------------------------------------------------------------------------

function wxfb.DebugDialog(parent)

    local self = wx.wxFrame ( parent, wx.wxID_ANY, "Lua Debug", wx.wxDefaultPosition, wx.wxSize( 400,600 ), wx.wxDEFAULT_FRAME_STYLE+wx.wxFRAME_FLOAT_ON_PARENT+wx.wxFRAME_TOOL_WINDOW+wx.wxTAB_TRAVERSAL )
    
    self:SetSizeHints( wx.wxDefaultSize, wx.wxDefaultSize )
    
    local bSizer1 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.panel = wx.wxPanel( self, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxTAB_TRAVERSAL )
    local bSizer2 = wx.wxBoxSizer( wx.wxVERTICAL )
    
    self.text = wx.wxTextCtrl( self.panel, wx.wxID_ANY, "", wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxHSCROLL+wx.wxTE_MULTILINE )
    self.text:SetFont( wx.wxFont( 8, 75, 90, 90, false, "Consolas" ) )
    
    bSizer2:Add( self.text, 1, wx.wxALL+wx.wxEXPAND, 5 )
    
    
    self.panel:SetSizer( bSizer2 )
    self.panel:Layout()
    bSizer2:Fit( self.panel )
    bSizer1:Add( self.panel, 1, wx.wxEXPAND, 5 )
    
    
    self:SetSizer( bSizer1 )
    self:Layout()
    
    self:Centre( wx.wxBOTH )
    
    -- Connect Events
    self:Connect(wx.wxEVT_CLOSE_WINDOW, function(evt) self:Hide(evt) end)

    return self
end

return wxfb