
%include "../puz/XPuzzle.hpp"
%include "../puz/XGrid.hpp"
%include "../puz/XSquare.hpp"

%class %noclassinfo %encapsulate XPuzzle
    XPuzzle(const wxString & filename = wxEmptyString)

    bool Load(const wxString & filename, wxString ext = wxEmptyString)
    bool Save(const wxString & filename, wxString ext = wxEmptyString)

    void Clear();
    bool IsOk()        const
    void SetOk(bool doit=true)
    bool IsScrambled() const
    short GetVersion() const

    const wxString & GetFilename() const
    void SetFilename(const wxString & filename)

    bool GetModified() const 
    void SetModified(bool modified) 

    int GetTime() const 
    void SetTime(int time) 

    const wxString & GetTitle() const 
    void SetTitle(const wxString & title) 

    const wxString & GetAuthor() const 
    void SetAuthor(const wxString & author) 

    const wxString & GetCopyright() const 
    void SetCopyright(const wxString & copyright) 

    const wxString & GetNotes() const 
    void SetNotes(const wxString & notes) 
/*
    XPuzzle::ClueList & GetAcross() 
    void SetAcross(const ClueList & across) 

    XPuzzle::ClueList & GetDown() 
    void SetDown(const ClueList & down) 

    std::vector<wxString> & GetClues() 
    void SetClues(const std::vector<wxString> & clues) 
*/
    // XGrid & GetGrid() 
    // void SetGrid(const XGrid & grid) 

%endclass
