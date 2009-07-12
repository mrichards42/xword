#include "XPuzzle.hpp"


inline bool
XPuzzle::Load(const wxString & filename, const wxString & ext)
{
    m_filename = filename;
    m_modified = false;
    m_isOk = PuzLoader::Load(this, filename, ext);
    return m_isOk;
}


inline bool
XPuzzle::Save(const wxString & filename, const wxString & ext)
{
    m_filename = filename;
    m_modified = false;
    return PuzLoader::Save(this, filename, ext);
}