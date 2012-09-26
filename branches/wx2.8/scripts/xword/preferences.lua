-- ============================================================================
-- Preferences panels
--      This script adds panels to the preferences dialog
-- ============================================================================

-- Use this function to add a panel to the preferences dialog
xword.preferences_panels = {}
function xword.AddPreferencesPanel(title, constructor, save_func)
    assert(title and constructor and save_func)
    table.insert(xword.preferences_panels,
        {title = title, constructor = constructor, save_func = save_func})
end

-- This is called from C++ from PreferencesDialog::OnInit
function xword.OnInitPreferencesDialog(notebook)
    for _, info in ipairs(xword.preferences_panels) do
        info.ctrl = info.constructor(notebook)
        notebook:AddPage(info.ctrl, info.title)
    end
end

-- This is called from C++ from PreferencesDialog::SaveConfig
function xword.OnSavePreferences()
    for _, info in ipairs(xword.preferences_panels) do
        info.save_func(info.ctrl)
    end
end