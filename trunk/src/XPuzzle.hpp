#ifndef X_PUZZLE_H
#define X_PUZZLE_H

#include <wx/string.h>
#include <vector>
#include "XGrid.hpp"
#include "PuzLoader.hpp"

class XPuzzle
{
public:
	explicit XPuzzle(const wxString & filename = wxEmptyString)
    {
        m_isOk = false;
        if (! filename.empty())
            Load(filename);
    }

	~XPuzzle() {}

	bool Load(const wxString & filename, const wxString & ext = wxEmptyString);
	bool Save(const wxString & filename, const wxString & ext = wxEmptyString);

	void Clear();
	bool IsOk() { return m_isOk; }

	wxString m_filename;
	bool m_modified;

	int m_time;
	bool m_complete;

	class Clue;
	typedef std::vector<Clue> ClueList;

	wxString m_title;
	wxString m_author;
	wxString m_copyright;
	wxString m_notes;

	// m_clues holds the actual clues
	std::vector<wxString> m_clues;

	// m_across and m_down hold copies of the clues (ref-counted)
	ClueList m_across;
	ClueList m_down;

	XGrid m_grid;

	// Extra random sections . . . just save them and append to the file later
	std::vector<wxString> m_extraSections;

protected:
	bool m_isOk;
};


// This just makes access to the clues nicer:
class XPuzzle::Clue
{
public:
	Clue(int num, const wxString & str)
		: m_num(num), m_str(str)
	{}

	const wxString & Text()   const { return m_str; }
	      wxString & Text()         { return m_str; }
	      int        Number() const { return m_num; }

	int m_num;
	wxString m_str;
	bool operator==(const XPuzzle::Clue & other) const
		{ return other.m_num == m_num && other.m_str == m_str; }
};




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


inline void
XPuzzle::Clear()
{
	m_grid.SetSize(0, 0);
	m_clues.clear();
	m_across.clear();
	m_down.clear();
	m_title.clear();
	m_author.clear();
	m_copyright.clear();
	m_notes.clear();
	m_time = 0;
	m_complete = false;
	m_extraSections.clear();
}

#endif // X_PUZZLE_H
