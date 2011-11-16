#include <wx/html/forcelnk.h>
#include <wx/html/m_templ.h>

// Define a custom tag <WBR> that allows a line break in the middle of a word
// This is accomplished by adding a blank wxHtmlWordCell and leaving the
// default value for wxHtmlWordCell::m_allowLinebreak alone (the default is
// true).
// When wxHtmlWinParser adds a wxHtmlWordCell, it tells the new word what the
// contents of the last word was; if the last word doesn't end in a space,
// the m_allowLinebreak is set to false.

TAG_HANDLER_BEGIN(WBR, "WBR")
    TAG_HANDLER_CONSTR(WBR) { }

    TAG_HANDLER_PROC(tag)
    {
        // Add an empty word.
        m_WParser->GetContainer()->InsertCell(
            new wxHtmlWordCell(wxEmptyString, *(m_WParser->GetDC()))
        );
        return false; // ParseInner not called
    }
TAG_HANDLER_END(WBR)

TAGS_MODULE_BEGIN(WordBreaks)
    TAGS_MODULE_ADD(WBR)
TAGS_MODULE_END(WordBreaks)
