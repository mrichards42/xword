// This file is part of XWord
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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

#include "preferences.hpp"
#include <wx/osx/private.h>
#include <wx/sizer.h>


void PreferencesDialog::DoMoveWindow(int x, int y, int width, int height)
{
    if ( m_nowpeer == NULL )
        return;
    
    m_cachedClippedRectValid = false ;
    
    // m_nowpeer->MoveWindow(x, y, width, height);
    // From wxNonOwnedWindowCocoaImpl::MoveWindow
    NSRect r = wxToNSRect( NULL, wxRect(x,y,width, height) );
    // do not trigger refreshes upon invisible and possible partly created objects
    [m_nowpeer->GetWXWindow() setFrame:r display:IsShownOnScreen() animate:YES];


    wxWindowMac::MacSuperChangedPosition() ; // like this only children will be notified

}
