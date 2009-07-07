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


#ifndef PUZ_LOADER_H
#define PUZ_LOADER_H

#include <wx/string.h>
#include <vector>

// forward declaration
class XPuzzle;

// A namespace to encapsulate loading and saving puzzle files
// This works like a class with two static functions
namespace PuzLoader
{
    bool Load(XPuzzle * puz,
              const wxString & filename,
              wxString ext = wxEmptyString);

    bool Save(XPuzzle * puz,
              const wxString & filename,
              wxString ext = wxEmptyString);

    bool CanSave(const wxString & ext);
    bool CanLoad(const wxString & ext);

    struct TypeDesc {
        wxString ext;
        wxString description;
    };

    // Spits out a string that can be passed to wxWidgets file dialogs
    std::vector<TypeDesc> GetSaveTypes();
    std::vector<TypeDesc> GetLoadTypes();
};

#endif // PUZ_LOADER_H