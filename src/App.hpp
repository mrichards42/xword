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


#ifndef MY_APP_H
#define MY_APP_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/app.h>
#endif

#include "utils/ConfigManager.hpp"

class MyFrame;
class wxPrintData;
class wxPageSetupDialogData;

// Global printing variables
extern wxPrintData * g_printData;
extern wxPageSetupDialogData* g_pageSetupData;


class MyApp : public wxApp
{
    friend class MyFrame; // So we can set m_frame to NULL when it is deleted
public:
    virtual bool OnInit();
    virtual bool ReadCommandLine();

    // return code = number of unsuccessful conversions
    virtual int OnRun() { wxApp::OnRun(); return m_retCode; }

    virtual int OnExit();

    // Get and set global data from the whole app
    int  GetReturnCode()  const  { return m_retCode; }
    void SetReturnCode(int code) { m_retCode = code; }

    ConfigManager & GetConfigManager() { return m_config; }
    wxString GetConfigPath();

private:
    void OnActivate(wxActivateEvent & evt);

    void SetupConfig();
    void SetupPrinting();

    MyFrame * m_frame;
    ConfigManager m_config;
    int m_retCode;
    bool m_isTimerRunning;

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(MyApp)

#endif // MY_APP_H
