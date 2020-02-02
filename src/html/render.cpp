#include "wx/dc.h"
#include "render.hpp"
#include "wx/wxhtml.h"

//--------------------------------------------------------------------------------
// MyHtmlDCRenderer
//--------------------------------------------------------------------------------


MyHtmlDCRenderer::MyHtmlDCRenderer(wxHtmlWinParser * parser) : wxObject()
{
    m_DC = NULL;
    m_Width = m_Height = 0;
    m_Cells = NULL;
    if (parser)
        m_Parser = parser;
    else
        m_Parser = new wxHtmlWinParser();
    m_FS = new wxFileSystem();
    m_Parser->SetFS(m_FS);
    SetStandardFonts(12);
}



MyHtmlDCRenderer::~MyHtmlDCRenderer()
{
    if (m_Cells) delete m_Cells;
    if (m_Parser) delete m_Parser;
    if (m_FS) delete m_FS;
}



void MyHtmlDCRenderer::SetDC(wxDC *dc, double pixel_scale)
{
    m_DC = dc;
    m_Parser->SetDC(m_DC, pixel_scale);
}



void MyHtmlDCRenderer::SetSize(int width, int height)
{
    m_Width = width;
    m_Height = height;
}


void MyHtmlDCRenderer::SetHtmlText(const wxString& html, const wxString& basepath, bool isdir)
{
    if (m_DC == NULL) return;

    if (m_Cells != NULL) delete m_Cells;

    m_FS->ChangePathTo(basepath, isdir);
    m_Cells = (wxHtmlContainerCell*) m_Parser->Parse(html);
    m_Cells->SetIndent(0, wxHTML_INDENT_ALL, wxHTML_UNITS_PIXELS);
    m_Cells->Layout(m_Width);
}


void MyHtmlDCRenderer::SetFonts(const wxString& normal_face, const wxString& fixed_face,
                                const int *sizes)
{
    m_Parser->SetFonts(normal_face, fixed_face, sizes);
    if (m_DC == NULL && m_Cells != NULL)
        m_Cells->Layout(m_Width);
}

void MyHtmlDCRenderer::SetStandardFonts(int size,
                                        const wxString& normal_face,
                                        const wxString& fixed_face)
{
    m_Parser->SetStandardFonts(size, normal_face, fixed_face);
    if (m_DC == NULL && m_Cells != NULL)
        m_Cells->Layout(m_Width);
}

int MyHtmlDCRenderer::Render(int x, int y,
                             int from, int dont_render, int to)
{
    int pbreak, hght;

    if (m_Cells == NULL || m_DC == NULL) return 0;

    pbreak = (int)(from + m_Height);
    while (m_Cells->AdjustPagebreak(&pbreak, m_Height)) {}
    hght = pbreak - from;
    if(to < hght)
        hght = to;

    if (!dont_render)
    {
        wxHtmlRenderingInfo rinfo;
        wxDefaultHtmlRenderingStyle rstyle;
        rinfo.SetStyle(&rstyle);
        m_DC->SetBrush(*wxWHITE_BRUSH);
        m_DC->SetClippingRegion(x, y, m_Width, hght);
        m_Cells->Draw(*m_DC,
                      x, (y - from),
                      y, y + hght,
                      rinfo);
        m_DC->DestroyClippingRegion();
    }

    if (pbreak < m_Cells->GetHeight()) return pbreak;
    else return GetTotalHeight();
}


int MyHtmlDCRenderer::GetTotalHeight()
{
    if (m_Cells) return m_Cells->GetHeight();
    else return 0;
}

int MyHtmlDCRenderer::GetTotalWidth()
{
    if (m_Cells) return m_Cells->GetWidth();
    else return 0;
}

// This hack forces the linker to always link in m_* files
// (wxHTML doesn't work without handlers from these files)
#include "wx/html/forcelnk.h"
FORCE_WXHTML_MODULES()
