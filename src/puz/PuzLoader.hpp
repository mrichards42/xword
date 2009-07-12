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

//------------------------------------------------------------------------------
// PuzLoader: a namespace to encapsulate loading and saving crossword puzzles.
//------------------------------------------------------------------------------

#ifndef PUZ_LOADER_H
#define PUZ_LOADER_H

#include <wx/string.h>
#include <vector>

class XPuzzle;

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

// Return a vector of extensions (without the '.')
std::vector<wxString> GetSaveTypes();
std::vector<wxString> GetLoadTypes();

// Return a a string compatible with the wxWidgets file dialogs:
// "File type 1 (*.ext)|*.ext | File type 2 (*.ext)|*.ext " . . . etc.
wxString GetSaveTypeString();
wxString GetLoadTypeString();

} // namespace PuzLoader

#endif // PUZ_LOADER_H
