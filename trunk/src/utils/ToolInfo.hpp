// This file is part of XWord
// Copyright (C) 2009 Mike Richards ( mrichards42@gmx.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#ifndef TOOL_INFO_H
#define TOOL_INFO_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

// TODO:
//   Better support for wxAuiToolBar

#include <wx/aui/auibar.h>
//#include "../widgets/MyAuiToolBar.hpp"

#include <boost/foreach.hpp>

#include <set>

class ToolManager;

// Class to hold tool information (all inlined)
class ToolInfo
{
    friend class ToolManager;
public:
    ToolInfo(int id,
             const wxString & label,
             const wxString & icoName1,
             const wxString & icoName2,
             const wxString & helpStr = wxEmptyString,
             wxItemKind kind = wxITEM_NORMAL);

    ToolInfo(int id,
             const wxString & label,
             const wxString & icoName1,
             const wxString & helpStr = wxEmptyString,
             wxItemKind kind = wxITEM_NORMAL);

    ~ToolInfo() {}

    // Set tool information (chaining allowed)
    //----------------------------------------
    ToolInfo & SetLabel      (const wxString & label);
    ToolInfo & SetHelpString (const wxString & helpStr);
    ToolInfo & SetIconName   (const wxString & icoName, bool checked = true);


    // Everything past here can also be accessed from ToolManager
    //-----------------------------------------------------------


    // Get tool information
    //---------------------
    int              GetId()         const { return m_id; }
    const wxString & GetLabel()      const { return m_label; }
          wxString   GetLabelText()  const { return wxStripMenuCodes(m_label); }
    const wxString & GetHelpString() const { return m_helpStr; }
    wxItemKind       GetKind()       const { return m_kind; }

    const wxString & GetIconName(bool checked = true) const
        { return checked ? m_icoName1 : m_icoName2; }

    bool HasBitmap(bool checked = true) const
        { return ! (checked ? m_icoName1.empty() : m_icoName2.empty()); }

    wxIcon   GetIcon  (int width, int height, bool checked = true) const;
    wxBitmap GetBitmap(int width, int height, bool checked = true) const;

    bool IsAttached(const wxMenu * menu)     const
        { return menu->FindItem(m_id) != NULL; }

    bool IsAttached(const wxToolBar * tb)    const
        { return tb->FindById(m_id) != NULL; }

    bool IsAttached(const wxAuiToolBar * tb) const
        { return tb->FindTool(m_id) != NULL; }


    // Interact with the tool
    //-----------------------
    ToolInfo & Check(bool checked = true);
    bool Toggle () { Check(! m_checked); return IsChecked(); }
    bool IsChecked() const { return m_checked; }

    ToolInfo & Enable(bool enabled = true);
    ToolInfo & Disable()   { return Enable(false); }
    bool IsEnabled() const { return m_enabled; }


    // Add the tool to menus / toolbars
    //---------------------------------
    wxMenuItem *        Add(wxMenu * menu,     int width = -1, int height = -1);
    wxToolBarToolBase * Add(wxToolBar * tb,    int width = -1, int height = -1);
    void                Add(wxAuiToolBar * tb, int width = -1, int height = -1);

    // Insert the tool to menus / toolbars
    //------------------------------------
    wxMenuItem        * Insert(wxMenu * menu,
                               size_t pos,
                               int width = -1,
                               int height = -1);

    wxToolBarToolBase * Insert(wxToolBar * tb,
                               size_t pos,
                               int width = -1,
                               int height = -1);

    // wxAuitoolBar doesn't have an Insert method . . .


    // Remove the tool from menus / toolbars
    //------------------------------------
    wxMenuItem *        Remove(wxMenu * menu);
    wxToolBarToolBase * Remove(wxToolBar * tb);
    // wxAuitoolBar doesn't have an Remove method . . .


    // Delete the tool from menus / toolbars
    //------------------------------------
    void                Delete(wxMenu * menu);
    bool                Delete(wxToolBar * tb);
    bool                Delete(wxAuiToolBar * tb);


private:
    int m_id;
    wxString m_label;
    wxString m_icoName1;
    wxString m_icoName2;
    wxString m_helpStr;
    wxItemKind m_kind;
    bool m_checked;
    bool m_enabled;

    // Keep lists of where this tool is attached
    // We need to keep pointers to the actual toolbar not the tools, because
    // of the way that updating tools is handled
    std::set<wxAuiToolBar *>  m_auiToolbars;
    std::set<wxToolBar *>     m_toolbars;
    std::set<wxMenuItem *>    m_menuItems;

};

#endif // TOOL_INFO_H
