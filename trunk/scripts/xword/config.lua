-- ============================================================================
-- Preferences panels
--      This script adds panels to the preferences dialog
-- ============================================================================

-- Use this function to add a panel to the preferences dialog
xword.preferences_panels = {}
function xword.AddPreferencesPanel(name, constructor)
    table.insert(xword.preferences_panels, {name, constructor})
end

-- This is called from C++ from PreferencesDialog::OnInit
function xword.OnInitPreferencesDialog(notebook)
    for _, info in ipairs(xword.preferences_panels) do
        local name, constructor = unpack(info)
        notebook:AddPage(constructor(notebook), name)
    end
end