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


#ifndef MY_DRAG_AND_DROP_H
#define MY_DRAG_AND_DROP_H

#include <wx/dnd.h>
#include "../MyFrame.hpp"


// The FileDropTarget class
//-------------------------
class XWordFileDropTarget : public wxFileDropTarget
{
public:
    XWordFileDropTarget(MyFrame * frame)
        : m_frame(frame)
    {}

    virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxArrayString & filenames)
    {
        if ( ! m_frame->ClosePuzzle(true) ) // Prompt for save
            return false;
        m_frame->LoadPuzzle(filenames.Item(0));
        return true;
    }

private:
    MyFrame * m_frame;
};


#endif // MY_DRAG_AND_DROP_H
