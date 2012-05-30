// This file is part of XWord    
// Copyright (C) 2012 Mike Richards ( mrichards42@gmx.com )
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


#ifndef STYLE_PANEL_H
#define STYLE_PANEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/treectrl.hpp>
#endif

#include "../paths.hpp"
#include "../config.hpp"

// ---------------------------------------------------------------------------
// Basic style panels
// ---------------------------------------------------------------------------

// The base class for all style panels
class StyleBase : public wxTreeItemData
{
public:
    StyleBase() {}
    virtual ~StyleBase() {}

    wxWindow * NewStylePanel(wxWindow * parent)
        { m_panel = MakeStylePanel(parent); return m_panel; }
    virtual void LoadConfig()=0;
    virtual void SaveConfig()=0;

    wxWindow * GetStylePanel() { return m_panel; }
    const wxWindow * GetStylePanel() const { return m_panel; }

protected:
    virtual wxWindow * MakeStylePanel(wxWindow * parent)=0;
    wxWindow * m_panel;
};

//----------------------------------------------------------------------------
// A basic style panel
//      Includes font, colors, and alignment
//----------------------------------------------------------------------------
enum BasicStyleFlag
{
    // FP_[...] styles are valid here
    STYLE_FONT             = FP_DEFAULT,
    STYLE_FONT_SIMPLE      = FP_DEFAULT & ~FP_POINTSIZE,


    // FP styles end at 1 << 4
    STYLE_TEXT_COLOR       = 1 << 5,
    STYLE_BACKGROUND_COLOR = 1 << 6,

    // Alignment
    STYLE_VERTICAL_ALIGN   = 1 << 7,
    STYLE_HORIZONTAL_ALIGN = 1 << 8,

    STYLE_ALIGN     = STYLE_HORIZONTAL_ALIGN | STYLE_VERTICAL_ALIGN,
    STYLE_TEXT      = STYLE_FONT | STYLE_TEXT_COLOR,
    STYLE_FONT_MASK = STYLE_FONT,
    STYLE_COLOR     = STYLE_TEXT_COLOR | STYLE_BACKGROUND_COLOR,
    STYLE_DEFAULT   = STYLE_FONT | STYLE_TEXT_COLOR | STYLE_BACKGROUND_COLOR,
    STYLE_ALL       = STYLE_DEFAULT | STYLE_ALIGN
};

#include "colorchoice.hpp"
class BasicStyle : public StyleBase
{
public:
    BasicStyle(long flag = STYLE_DEFAULT)
        : m_flag(flag)
    {}

    virtual ~BasicStyle() {}

    int GetAlignment()
    {
        int align = 0;
        if (m_flag & STYLE_HORIZONTAL_ALIGN)
        {
            if (m_center->GetValue())
                align |= wxALIGN_CENTER_HORIZONTAL;
            else if (m_right->GetValue())
                align |= wxALIGN_RIGHT;
        }
        if (m_flag & STYLE_VERTICAL_ALIGN)
        {
            if (m_middle->GetValue())
                align |= wxALIGN_CENTER_VERTICAL;
            else if (m_bottom->GetValue())
                align |= wxALIGN_BOTTOM;
        }
        return align;
    }

    void SetAlignment(int align)
    {
        if (m_flag & STYLE_HORIZONTAL_ALIGN)
        {
            if (align & wxALIGN_CENTER_HORIZONTAL)
                m_center->SetValue(true);
            else if (align & wxALIGN_RIGHT)
                m_right->SetValue(true);
        }
        if (m_flag & STYLE_VERTICAL_ALIGN)
        {
            if (align & wxALIGN_CENTER_VERTICAL)
                m_middle->SetValue(true);
            else if (align & wxALIGN_BOTTOM)
                m_bottom->SetValue(true);
        }
    }

protected:
    long m_flag;
    FontPanel * m_font;
    ColorChoice * m_textColor;
    ColorChoice * m_bgColor;

    wxRadioButton * m_left;
    wxRadioButton * m_center;
    wxRadioButton * m_right;

    wxRadioButton * m_top;
    wxRadioButton * m_middle;
    wxRadioButton * m_bottom;

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        wxPanel * panel = new wxPanel(parent, wxID_ANY);
        wxSizer * sizer = new wxFlexGridSizer(0, 2, 5, 5);
        // Font
        if (m_flag & STYLE_FONT_MASK)
        {
            m_font = new FontPanel(panel, wxID_ANY, wxNullFont, m_flag & STYLE_FONT_MASK);
            sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Font:")), 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(m_font, 0, wxALIGN_CENTER_VERTICAL);
        }
        // Alignment
        wxSizer * align_sizer;
        if (m_flag & STYLE_HORIZONTAL_ALIGN)
        {
            m_left = new wxRadioButton(panel, wxID_ANY, _T("Left"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_center = new wxRadioButton(panel, wxID_ANY, _T("Center"));
            m_right = new wxRadioButton(panel, wxID_ANY, _T("Right"));
            wxSizer * halign_sizer = new wxGridSizer(0, 3, 5, 5);
            halign_sizer->Add(m_left);
            halign_sizer->Add(m_center);
            halign_sizer->Add(m_right);
            sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Alignment:")), 0, wxALIGN_CENTER_VERTICAL);
            if ((m_flag & STYLE_VERTICAL_ALIGN) == 0)
            {
                sizer->Add(halign_sizer, 0, wxALIGN_CENTER_VERTICAL);
            }
            else
            {
                // If we have both alignments, wrap each in a static box
                align_sizer = new wxBoxSizer(wxVERTICAL);
                wxSizer * sbsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Horizontal"));
                sbsizer->Add(halign_sizer, 0, wxALL, 3);
                align_sizer->Add(sbsizer, 0, wxBOTTOM, 5);
                sizer->Add(align_sizer, 0, wxALIGN_CENTER_VERTICAL);
            }
        }
        if (m_flag & STYLE_VERTICAL_ALIGN)
        {
            m_top = new wxRadioButton(panel, wxID_ANY, _T("Top"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_middle = new wxRadioButton(panel, wxID_ANY, _T("Middle"));
            m_bottom = new wxRadioButton(panel, wxID_ANY, _T("Bottom"));
            wxSizer * valign_sizer = new wxGridSizer(0, 3, 5, 5);
            valign_sizer->Add(m_top);
            valign_sizer->Add(m_middle);
            valign_sizer->Add(m_bottom);

            if ((m_flag & STYLE_HORIZONTAL_ALIGN) == 0)
            {
                sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Alignment:")), 0, wxALIGN_CENTER_VERTICAL);
                sizer->Add(valign_sizer, 0, wxALIGN_CENTER_VERTICAL);
            }
            else
            {
                // If we have both alignments, wrap each in a static box
                wxSizer * sbsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Vertical"));
                sbsizer->Add(valign_sizer, 0, wxALL, 3);
                align_sizer->Add(sbsizer);
            }
        }
        // Colors
        if (m_flag & STYLE_TEXT_COLOR)
        {
            m_textColor = new ColorChoice(panel, wxID_ANY);
            sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Text Color:")), 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(m_textColor, 0, wxALIGN_CENTER_VERTICAL);
        }
        if (m_flag & STYLE_BACKGROUND_COLOR)
        {
            m_bgColor = new ColorChoice(panel, wxID_ANY);
            sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Background:")), 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(m_bgColor, 0, wxALIGN_CENTER_VERTICAL);
        }
        panel->SetSizer(sizer);
        return panel;
    }
};

//----------------------------------------------------------------------------
// A metadata ctrl
//      Includes the basics plus display format.
//----------------------------------------------------------------------------

#include "../MetadataCtrl.hpp"

class DisplayFormatDialog : public DisplayFormatDialogBase
{
public:
    DisplayFormatDialog(wxWindow * parent, wxWindowID id = wxID_ANY)
        : DisplayFormatDialogBase(parent, id)
    {}

    // Hide or show the function wxStaticText
    void UpdateDisplay()
    {
        const bool isChecked = m_useLua->GetValue();
        m_sizer->Show(m_functionStart, isChecked);
        m_sizer->Show(m_functionEnd, isChecked);
        m_sizer->GetItem(m_format)->SetBorder(isChecked ? 15 : 0);
        Layout();
    }

    void UpdateResult()
    {
        MyFrame * frame = wxDynamicCast(GetParent()->GetParent(), MyFrame);
        wxString value = MetadataCtrl::FormatLabel(m_format->GetValue(), frame, m_useLua->GetValue());
        // Tool tip is the whole result
        m_result->SetToolTip(value);
        // Label is the first line
        wxString label = value.BeforeFirst(_T('\n'));
        if (label.size() != value.size())
            label << _T("...");
        m_result->SetLabel(label);
    }

protected:
    void OnTextUpdated(wxCommandEvent & evt)
    {
        UpdateResult();
        evt.Skip();
    }

    void OnUseLua(wxCommandEvent & evt)
    {
        UpdateDisplay();
        UpdateResult();
        evt.Skip();
    }
};


class MetadataStyleBase : public wxEvtHandler, public BasicStyle
{
public:
    MetadataStyleBase(long flag = STYLE_ALL)
        : BasicStyle(flag)
    {}

    virtual ~MetadataStyleBase() {}

protected:
    wxTextCtrl * m_format;
    wxCheckBox * m_useLua;

    virtual wxArrayString GetMetadataFields()
    {
        const wxChar * fields[] = {
            _T("title"),
            _T("author"),
            _T("copyright"),
            _T("editor"),
            _T("publisher"),
            _T("date")
        };
        return wxArrayString(sizeof(fields) / sizeof(wxChar*), fields);
    }

    void OnFormatHelp(wxCommandEvent & WXUNUSED(evt))
    {
        wxWindow * prefs = wxGetTopLevelParent(GetStylePanel());
        MetadataFormatHelpDialog * dlg = new MetadataFormatHelpDialog(prefs);

        // Create the metadata fields
        wxSizer * sizer = dlg->m_sizer;
        wxArrayString fields = GetMetadataFields();
        MyFrame * frame = wxDynamicCast(prefs->GetParent(), MyFrame);
        for (size_t i = 0; i < fields.Count(); ++i)
        {
            wxString field = fields.Item(i);
            wxStaticText * text = new wxStaticText(dlg->m_panel, wxID_ANY, _T("%") + field + _T("%"));
            text->SetFont(wxFont(-1, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("consolas")));
            sizer->Add(text);
            wxString meta = MetadataCtrl::GetMeta(field, frame);
            sizer->Add(new wxStaticText(dlg->m_panel, wxID_ANY, meta));
        }

        dlg->Fit();
        dlg->Show();
    }

    void OnExpandText(wxCommandEvent & WXUNUSED(evt))
    {
        wxWindow * prefs = wxGetTopLevelParent(GetStylePanel());
        DisplayFormatDialog dlg(prefs, wxID_ANY);
        dlg.m_format->SetValue(m_format->GetValue());
        dlg.m_useLua->SetValue(m_useLua->GetValue());
#if ! (XWORD_USE_LUA)
        dlg.m_useLua->SetValue(false);
        dlg.m_useLua->GetContainingSizer()->Hide(dlg.m_useLua);
        dlg.Layout();
#endif // XWORD_USE_LUA
        dlg.UpdateDisplay();
        dlg.UpdateResult();
        if (dlg.ShowModal() == wxID_OK)
        {
            m_format->SetValue(dlg.m_format->GetValue());
            m_useLua->SetValue(dlg.m_useLua->GetValue());
        }
    }

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        wxWindow * panel = BasicStyle::MakeStylePanel(parent);
        wxFlexGridSizer * sizer = dynamic_cast<wxFlexGridSizer *>(panel->GetSizer());

        // Add display format
        m_format = new wxTextCtrl(panel, wxID_ANY, wxEmptyString,
            wxDefaultPosition,wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP);
        m_format->SetFont(wxFont(-1, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("consolas")));

        wxBoxSizer * leftSizer = new wxBoxSizer(wxVERTICAL);
        leftSizer->Add(new wxStaticText(panel, wxID_ANY, _T("Display\nFormat:")));

        m_useLua = new wxCheckBox(panel, wxID_ANY, _T("Script"));
        m_useLua->SetToolTip(_T("Format as a lua script"));
#if XWORD_USE_LUA
        leftSizer->Add(m_useLua, 0, wxTOP, 5);
#else // ! XWORD_USE_LUA
        m_useLua->Hide();
#endif // XWORD_USE_LUA

        // Help button
        wxBitmapButton * help = new wxBitmapButton(panel, wxID_ANY, LoadXWordBitmap(_T("help"), 16));
        help->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MetadataStyleBase::OnFormatHelp), NULL, this);
        help->SetToolTip(_T("Format help"));

        // Expand button
        wxBitmapButton * expand = new wxBitmapButton(panel, wxID_ANY, LoadXWordBitmap(_T("expand"), 16));
        expand->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MetadataStyleBase::OnExpandText), NULL, this);
        expand->SetToolTip(_T("Show edit dialog"));

        wxSizer * btnSizer = new wxBoxSizer(wxHORIZONTAL);
        btnSizer->Add(help, 0, wxRIGHT, 5);
        btnSizer->Add(expand);
        leftSizer->Add(btnSizer, 0, wxTOP | wxALIGN_BOTTOM, 5);

        sizer->Add(leftSizer);
        sizer->Add(m_format, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
        sizer->AddGrowableRow(sizer->GetChildren().GetCount() / 2 - 1);

        sizer->AddGrowableCol(1);
        panel->Layout();
        return panel;
    }
};

void MetadataFormatHelpDialog::OnOk(wxCommandEvent & evt)
{
    Close();
}

void MetadataFormatHelpDialog::OnClose(wxCloseEvent & evt)
{
    Destroy();
    evt.Skip();
}





// ---------------------------------------------------------------------------
// Custom panels for each style
// ---------------------------------------------------------------------------

#include "../utils/color.hpp"

// Simple style
class SimpleStyle : public BasicStyle
{
public:
    SimpleStyle(ConfigManager & config)
        : BasicStyle(FP_FACENAME | STYLE_COLOR),
          m_config(config)
    {}

    void LoadConfig()
    {
        m_font->SetSelectedFont(m_config.Grid.letterFont());
        m_textColor->SetColor(m_config.Grid.penColor());
        m_bgColor->SetColor(m_config.Grid.backgroundColor());
        m_listHighlight->SetColor(m_config.Clue.selectedBackgroundColor());
        m_gridHighlight->SetColor(m_config.Grid.focusedLetterColor());
    }

    void SaveConfig()
    {
        // Fonts
        wxFont font = m_font->GetSelectedFont();
        SetFaceName(m_config.Clue.font, font);
        SetFaceName(m_config.Clue.headingFont, font);
        SetFaceName(m_config.Grid.letterFont, font);
        SetFaceName(m_config.Grid.numberFont, font);
        SetFaceName(m_config.CluePrompt.font, font);

        // Colors
        wxColour bgColor = m_textColor->GetColor();
        m_config.Clue.headingForegroundColor = bgColor;
        m_config.Clue.listForegroundColor = bgColor;
        m_config.Grid.penColor = bgColor;
        m_config.CluePrompt.foregroundColor = bgColor;

        wxColour fgColor = m_bgColor->GetColor();
        m_config.Clue.listBackgroundColor = fgColor;
        m_config.Clue.crossingBackgroundColor = fgColor;
        m_config.Clue.selectedForegroundColor = fgColor;
        m_config.Grid.backgroundColor = fgColor;
        m_config.CluePrompt.backgroundColor = fgColor;

        wxColour highlightColor = m_listHighlight->GetColor();
        m_config.Clue.selectedBackgroundColor = highlightColor;
        m_config.Clue.crossingForegroundColor = highlightColor;
        m_config.Grid.selectionColor = highlightColor;

        wxColour letterHighlight = m_gridHighlight->GetColor();
        m_config.Grid.focusedLetterColor = letterHighlight;
        m_config.Grid.focusedWordColor = GetWordHighlight(letterHighlight);

        // Metadata
        ConfigManager::MetadataCtrls_t & metadata = m_config.MetadataCtrls;
        ConfigManager::MetadataCtrls_t::iterator meta;
        for (meta = metadata.begin(); meta != metadata.end(); ++meta)
        {
            SetFaceName(meta->font, font);
            meta->backgroundColor = bgColor;
            meta->foregroundColor = fgColor;
        }
    }

protected:
    ConfigManager & m_config;

    // Helper functions
    static void SetFaceName(ConfigFont & cfg, const wxFont & font)
    {
        wxFont f(cfg.Get());
        f.SetFaceName(font.GetFaceName());
        cfg.Set(f);
    }

    static wxColour GetWordHighlight(const wxColour & letter)
    {
        // HSV isn't the best color model, but wxWidgets already has
        // conversion functions, and it's at least better than trying to
        // mess with RGB.
        wxImage::RGBValue rgb(letter.Red(), letter.Green(), letter.Blue());
        wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
        // Numbers here were established by trial and error.
        if (hsv.value < .3) // Dark colors
        {
            hsv.value += .2;
        }
        else if (hsv.saturation < .4) // Gray colors
        {
            if (hsv.saturation > .1) // If it's not too gray, saturate it
                hsv.saturation += .2;
            // Adjust value up or down
            if (hsv.value < .5)
                hsv.value += .2;
            else
                hsv.value -= .2;
        }
        else // "Colorful" colors (saturated and medium to high value)
        {
            // Adjust saturation up or down
            if (hsv.saturation > .5)
                hsv.saturation = std::max(0., hsv.saturation * .25);
            else
                hsv.saturation = std::min(1., hsv.saturation / .25);
            // Adjust value up or down
            if (hsv.value > .5)
                hsv.value = std::max(0., hsv.value * .9);
            else
                hsv.value = std::min(1., hsv.value / .9);
        }
        rgb = wxImage::HSVtoRGB(hsv);
        return wxColour(rgb.red, rgb.green, rgb.blue);
    }

    ColorChoice * m_listHighlight;
    ColorChoice * m_gridHighlight;

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        wxWindow * panel = BasicStyle::MakeStylePanel(parent);
        wxSizer * sizer = panel->GetSizer();

        m_listHighlight = new ColorChoice(panel, wxID_ANY);
        sizer->Add(new wxStaticText(panel, wxID_ANY, _T("List Highlight:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(m_listHighlight, 0, wxALIGN_CENTER_VERTICAL);

        m_gridHighlight = new ColorChoice(panel, wxID_ANY);
        sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Grid Highlight:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(m_gridHighlight, 0, wxALIGN_CENTER_VERTICAL);

        sizer->Layout();
        return panel;
    }
};


// Metadata
class MetadataStyle : public MetadataStyleBase
{
public:
    MetadataStyle(ConfigManager::Metadata_t & config)
        : MetadataStyleBase(FP_FACENAME | STYLE_ALIGN | STYLE_COLOR),
          m_config(config)
    {}

    void LoadConfig()
    {
        m_font->SetSelectedFont(m_config.font());
        SetAlignment(m_config.alignment());
        m_textColor->SetColor(m_config.foregroundColor());
        m_bgColor->SetColor(m_config.backgroundColor());
        m_format->SetValue(m_config.displayFormat());
        m_useLua->SetValue(m_config.useLua());
    }

    void SaveConfig()
    {
        m_config.font = m_font->GetSelectedFont();
        m_config.alignment = GetAlignment();
        m_config.foregroundColor = m_textColor->GetColor();
        m_config.backgroundColor = m_bgColor->GetColor();
        m_config.displayFormat = m_format->GetValue();
        m_config.useLua = m_useLua->GetValue();
    }

protected:
    ConfigManager::Metadata_t & m_config;
};

// Clue Prompt
class CluePromptStyle : public MetadataStyle
{
public:
    CluePromptStyle(ConfigManager::Metadata_t & config)
        : MetadataStyle(config)
    {}
protected:
    virtual wxArrayString GetMetadataFields()
    {
        const wxChar * fields[] = {
            _T("cluenumber"),
            _T("clue")
        };
        return wxArrayString(sizeof(fields) / sizeof(wxChar*), fields);
    }
};



// Clue List
class ClueListStyle : public BasicStyle
{
public:
    ClueListStyle(ConfigManager::Clue_t & config)
        : BasicStyle(STYLE_COLOR | FP_FACENAME | FP_POINTSIZE), m_config(config)
    {}

    void LoadConfig()
    {
        m_font->SetSelectedFont(m_config.font());
        m_textColor->SetColor(m_config.listForegroundColor());
        m_bgColor->SetColor(m_config.listBackgroundColor());
    }

    void SaveConfig()
    {
        m_config.font = m_font->GetSelectedFont();
        m_config.listForegroundColor = m_textColor->GetColor();
        m_config.listBackgroundColor = m_bgColor->GetColor();
    }

protected:
    ConfigManager::Clue_t & m_config;
};

// Clue List current clue
class ClueListSelectionStyle : public BasicStyle
{
public:
    ClueListSelectionStyle(ConfigManager::Clue_t & config)
        : BasicStyle(STYLE_COLOR), m_config(config)
    {}

    void LoadConfig()
    {
        m_textColor->SetColor(m_config.selectedForegroundColor());
        m_bgColor->SetColor(m_config.selectedBackgroundColor());
    }

    void SaveConfig()
    {
        m_config.selectedForegroundColor = m_textColor->GetColor();
        m_config.selectedBackgroundColor = m_bgColor->GetColor();
    }

protected:
    ConfigManager::Clue_t & m_config;
};

// Clue List crossing clue
class ClueListCrossingStyle : public BasicStyle
{
public:
    ClueListCrossingStyle(ConfigManager::Clue_t & config)
        : BasicStyle(STYLE_COLOR), m_config(config)
    {}

    void LoadConfig()
    {
        m_textColor->SetColor(m_config.crossingForegroundColor());
        m_bgColor->SetColor(m_config.crossingBackgroundColor());
    }

    void SaveConfig()
    {
        m_config.crossingForegroundColor = m_textColor->GetColor();
        m_config.crossingBackgroundColor = m_bgColor->GetColor();
    }

protected:
    ConfigManager::Clue_t & m_config;
};

// Clue List Heading
class ClueListHeadingStyle : public BasicStyle
{
public:
    ClueListHeadingStyle(ConfigManager::Clue_t & config)
        : BasicStyle(STYLE_COLOR | STYLE_FONT), m_config(config)
    {}

    void LoadConfig()
    {
        m_font->SetSelectedFont(m_config.headingFont());
        m_textColor->SetColor(m_config.headingForegroundColor());
        m_bgColor->SetColor(m_config.headingBackgroundColor());
    }

    void SaveConfig()
    {
        m_config.headingFont = m_font->GetSelectedFont();
        m_config.headingForegroundColor = m_textColor->GetColor();
        m_config.headingBackgroundColor = m_bgColor->GetColor();
    }

protected:
    ConfigManager::Clue_t & m_config;
};



// Grid
class GridBaseStyle : public BasicStyle
{
public:
    GridBaseStyle(ConfigManager::Grid_t & config)
        : BasicStyle(STYLE_FONT_SIMPLE | STYLE_COLOR), m_config(config)
    {}

    void LoadConfig()
    {
        m_font->SetSelectedFont(m_config.letterFont());
        m_numberFont->SetSelectedFont(m_config.numberFont());
        m_textColor->SetColor(m_config.penColor());
        m_bgColor->SetColor(m_config.backgroundColor());
    }

    void SaveConfig()
    {
        m_config.letterFont = m_font->GetSelectedFont();
        m_config.numberFont = m_numberFont->GetSelectedFont();
        m_config.penColor = m_textColor->GetColor();
        m_config.backgroundColor = m_bgColor->GetColor();
    }

protected:
    ConfigManager::Grid_t & m_config;
    FontPanel * m_numberFont;

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        wxWindow * panel = BasicStyle::MakeStylePanel(parent);
        wxSizer * sizer = panel->GetSizer();
        m_numberFont = new FontPanel(panel, wxID_ANY, wxNullFont, FP_DEFAULT & ~ FP_POINTSIZE);
        sizer->Insert(2, new wxStaticText(panel, wxID_ANY, _T("Number Font:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Insert(3, m_numberFont, 0, wxALIGN_CENTER_VERTICAL);
        sizer->Layout();
        return panel;
    }
};


// Grid Cursor/Selection
class GridSelectionStyle : public StyleBase
{
public:
    GridSelectionStyle(ConfigManager::Grid_t & config)
        : m_config(config)
    {}

    void LoadConfig()
    {
        m_letterColor->SetColor(m_config.focusedLetterColor());
        m_wordColor->SetColor(m_config.focusedWordColor());
        m_selectionColor->SetColor(m_config.selectionColor());
    }

    void SaveConfig()
    {
        m_config.focusedLetterColor = m_letterColor->GetColor();
        m_config.focusedWordColor = m_wordColor->GetColor();
        m_config.selectionColor = m_selectionColor->GetColor();
    }

protected:
    ConfigManager::Grid_t & m_config;

    ColorChoice * m_letterColor;
    ColorChoice * m_wordColor;
    ColorChoice * m_selectionColor;

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        wxPanel * panel = new wxPanel(parent, wxID_ANY);
        wxSizer * sizer = new wxFlexGridSizer(0, 2, 5, 5);

        m_letterColor = new ColorChoice(panel, wxID_ANY);
        sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Selected Letter:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(m_letterColor, 0, wxALIGN_CENTER_VERTICAL);

        m_wordColor = new ColorChoice(panel, wxID_ANY);
        sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Selected Word:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(m_wordColor, 0, wxALIGN_CENTER_VERTICAL);

        m_selectionColor = new ColorChoice(panel, wxID_ANY);
        sizer->Add(new wxStaticText(panel, wxID_ANY, _T("Multi-square Selection:")), 0, wxALIGN_CENTER_VERTICAL);
        sizer->Add(m_selectionColor, 0, wxALIGN_CENTER_VERTICAL);

        panel->SetSizer(sizer);
        return panel;
    }

};


// Grid Tweaks
class GridTweaksStyle : public StyleBase
{
public:
    GridTweaksStyle(ConfigManager::Grid_t & config)
        : m_config(config)
    {}

    virtual wxWindow * MakeStylePanel(wxWindow * parent)
    {
        m_panel = new GridTweaksPanel(parent);
        return m_panel;
    }

    void LoadConfig()
    {
        m_panel->m_lineThickness->SetValue(m_config.lineThickness());
        m_panel->m_letterScale->SetValue(m_config.letterScale());
        m_panel->m_numberScale->SetValue(m_config.numberScale());
    }

    void SaveConfig()
    {
        m_config.lineThickness = m_panel->m_lineThickness->GetValue();
        m_config.letterScale = m_panel->m_letterScale->GetValue();
        m_config.numberScale = m_panel->m_numberScale->GetValue();
    }

protected:
    ConfigManager::Grid_t & m_config;
    GridTweaksPanel * m_panel;
};

#endif // STYLE_PANEL_H
