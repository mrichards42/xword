-- ============================================================================
-- Preferences panels
--      This script adds panels to the preferences dialog
-- ============================================================================

xword.preferences_panels = {}

--- Add a custom preferences panel.
-- The panel should have a method panel:SaveConfig() that applies changes.
-- @param title The panel title
-- @param constructor The constructor.
function xword.AddPreferencesPanel(title, constructor)
    assert(title and constructor)
    table.insert(xword.preferences_panels,
        {title = title, constructor = constructor})
end

-- Remove a panel from the preferences dialog
function xword.RemovePreferencesPanel(title)
    for i, panel in ipairs(xword.preferences_panels) do
        if panel.title == title then
            table.remove(xword.preferences_panels, i)
            return true
        end
    end
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
        info.ctrl:SaveConfig()
    end
end