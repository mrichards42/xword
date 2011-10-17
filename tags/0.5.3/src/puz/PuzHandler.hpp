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


#ifndef PUZ_HANDLER_H
#define PUZ_HANDLER_H

#include "HandlerBase.hpp"

class PuzHandler : public HandlerBase
{
protected:
    void DoLoad();
    void DoSave();

    wxString GetExtension()   const { return _T("puz"); }
    wxString GetDescription() const { return _T("Across Lite format"); }
    bool CanLoad() const { return true; }
    bool CanSave() const { return true; }

    // Read extra sections
    //--------------------
    void LoadSections();

    void SetGEXT(const ByteArray & data);
    void SetLTIM(const ByteArray & data);
    void SetRUSR(const ByteArray & data);
    void SetSolutionRebus(const ByteArray & table, const ByteArray & grid);

    void AddSection(const wxString & name, const ByteArray & data)
        { m_puz->m_extraSections.push_back(XPuzzle::section(name, data)); }


    // Write extra sections
    //---------------------
    void WriteSections();

    void WriteGEXT();
    void WriteLTIM();
    void WriteRUSR();
    void WriteSolutionRebus();

    void WriteSection(const wxString & name, const ByteArray & data);
    void WriteSection(const XPuzzle::section & section)
        { WriteSection(section.name, section.data); }
};

#endif // PUZ_HANDLER_H