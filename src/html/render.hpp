#ifndef MY_HTML_RENDER
#define MY_HTML_RENDER

#include "wx/html/htmlcell.h"
#include "wx/html/winpars.h"
#include "wx/html/htmlfilt.h"

// This is a hacked-up version of wxHtmlDCRenderer

class MyHtmlDCRenderer : public wxObject
{
public:
    MyHtmlDCRenderer(wxHtmlWinParser * parser = NULL);
    virtual ~MyHtmlDCRenderer();

    // Following 3 methods *must* be called before any call to Render:

    // Assign DC to this render
    void SetDC(wxDC *dc, double pixel_scale = 1.0);

    // Sets size of output rectangle, in pixels. Note that you *can't* change
    // width of the rectangle between calls to Render! (You can freely change height.)
    void SetSize(int width, int height);

    // Sets the text to be displayed.
    // Basepath is base directory (html string would be stored there if it was in
    // file). It is used to determine path for loading images, for example.
    // isdir is false if basepath is filename, true if it is directory name
    // (see wxFileSystem for detailed explanation)
    void SetHtmlText(const wxString& html, const wxString& basepath = wxEmptyString, bool isdir = true);

    // Sets fonts to be used when displaying HTML page. (if size null then default sizes used).
    void SetFonts(const wxString& normal_face, const wxString& fixed_face, const int *sizes = NULL);

    // Sets font sizes to be relative to the given size or the system
    // default size; use either specified or default font
    void SetStandardFonts(int size = -1,
                          const wxString& normal_face = wxEmptyString,
                          const wxString& fixed_face = wxEmptyString);

    // [x,y] is position of upper-left corner of printing rectangle (see SetSize)
    // from is y-coordinate of the very first visible cell
    // to is y-coordinate of the next following page break, if any
    // Returned value is y coordinate of first cell than didn't fit onto page.
    // Use this value as 'from' in next call to Render in order to print multiple pages
    // document
    // If dont_render is TRUE then nothing is rendered into DC and it only counts
    // pixels and return y coord of the next page
    //
    // CAUTION! Render() changes DC's user scale and does NOT restore it!
    int Render(int x, int y, int from = 0,
               int dont_render = FALSE, int to = INT_MAX);

    // returns total height of the html document
    // (compare Render's return value with this)
    int GetTotalHeight();

    // returns width of the html document (if text is too long, it might
    // be larger than the width supplied to SetSize)
    int GetTotalWidth();

    wxHtmlWinParser * GetParser() { return m_Parser; }
    wxHtmlContainerCell * GetContainer() { return m_Cells; }

private:
    wxDC *m_DC;
    wxHtmlWinParser *m_Parser;
    wxFileSystem *m_FS;
    wxHtmlContainerCell *m_Cells;
    int m_MaxWidth, m_Width, m_Height;

    DECLARE_NO_COPY_CLASS(MyHtmlDCRenderer)
};

#endif // MY_HTML_RENDER
