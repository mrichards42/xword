#ifndef SIZER_XML_H
#define SIZER_XML_H


#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/window.h>
#    include <wx/sizer.h>
#endif


class SizerPrinter
{
public:
    static wxString Serialize(wxWindow * window, int tabs = 0);
    static wxString Serialize(wxSizer *  sizer,  int tabs = 0);

private:
    static wxString GetTabs(int tabs) { return wxString(_T(' '), tabs * 2); }
    static void     AddLine(wxString & str, int tabs, const wxString & text)
    {
        str.Append(GetTabs(tabs));
        str.Append(text);
        str.Append(_T("\n"));
    }

    wxWindow * m_window;
};



wxString
SizerPrinter::Serialize(wxWindow * window, int tabs)
{
    wxString ret;
    AddLine( ret, tabs, wxString::Format(_T("[%d] %s"),
                                         window->GetId(),
                                         window->GetClassInfo()->GetClassName()) );

    wxSizer * sizer = window->GetSizer();
    if (sizer != NULL)
        ret.Append(Serialize(sizer, tabs + 1));

    return ret;
}




wxString
SizerPrinter::Serialize(wxSizer * sizer, int tabs)
{
    wxString ret;
    AddLine(ret, tabs, sizer->GetClassInfo()->GetClassName());

    const wxSizerItemList & items = sizer->GetChildren();
    for (wxSizerItemList::const_iterator it = items.begin();
         it != items.end();
         ++it)
    {
        wxSizerItem * item = (*it);
        if      (item->IsWindow())
            ret.Append(Serialize(item->GetWindow(), tabs + 1));
        else if (item->IsSizer())
            ret.Append(Serialize(item->GetSizer(),  tabs + 1));
        else if (item->IsSpacer())
            AddLine(ret, tabs + 1, _T("[spacer]"));
    }
    return ret;
}

#endif // SIZER_XML_H