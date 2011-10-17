-- Defaults for the download package configuration.

if not download then download = {} end

-- Download output folder (created if it doesn't exist)
download.localfolder = xword.userdatadir.."/puzzles"

-- strftime format for column headers in the download dialog
download.dateformat = "%a, %b %d"

-- strftime format for puzzle files
download.puzformat = "%Y%m%d"

-- dialog size
download.dlgsize = { 500, 500 }