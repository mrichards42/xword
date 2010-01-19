-- ============================================================================
-- Download sources and configuration
-- ============================================================================

-- Create the download module table if it doesn't exist
if not download then download = {} end

-- Download output folder (created if it doesn't exist)
download.localfolder = [[D:\C++\XWord\trunk\scripts\download\puzzles]]

-- strftime format for the column headers in the download dialog
download.headerformat = '%a, %b %d'

--[[
Each entry in the sources table should consist of the following:
    display
        <string> the display name
    url
        <string> a string with strftime format codes
           or
        <function> a function recieving a date and returning a url
    filename
        <string> a string with strftime format codes for the local filename
            or
        <function> a function receiving a date and returning the local filename
    days
        <table> a table of boolean values indicating which days of the week to
                expect puzzle files.  The week starts with monday.
    format (optional)
        <string> a string indicating the file format as used by the "import"
                 add-on.  If no format is given, XWord will try to load the
                 puzzle as either Across Lite .puz or .txt format.
]]

download.sources = 
{
    {
        display = "CrosSynergy",
        url = "http://www.chron.com/apps/games/xword/puzzles/cs%y%m%d.puz",
        filename = "cs%Y%m%d.puz",
        days = { true, true, true, true, true, true, true },
    },

    {
        display = "Newsday",
        url = "http://picayune.uclick.com/comics/crnet/data/crnet%y%m%d-data.xml",
        filename = "nd%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "LA Times",
        url = "http://picayune.uclick.com/comics/tmcal/data/tmcal%y%m%d-data.xml",
        filename = "lat%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        filename = "usa%Y%m%d.xml",
        days = { true, true, true, true, true, false, false },
        format = 'XML (uclick)',
    },

    {
        display = "Universal",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        filename = "univ%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "Ink Well",
        url = "http://herbach.dnsalias.com/Tausig/vv%y%m%d.puz",
        filename = "tausig%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        display = "The Onion AV Club",
        url = "http://herbach.dnsalias.com/Tausig/av%y%m%d.puz",
        filename = "av%Y%m%d.puz",
        days = { false, false, true, false, false, false, false },
    },

    {
        display = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        filename = "jones%Y%m%d.puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        display = "Wall Street Journal",
        url = "http://mazerlm.home.att.net/wsj%y%m%d.puz",
        filename = "wsj%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        display = "Boston Globe",
        url = "http://home.comcast.net/~nshack/Puzzles/bg%y%m%d.puz",
        filename = "bg%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        display = "Philidelphia Inquirer",
        url = "http://mazerlm.home.att.net/pi%y%m%d.puz",
        filename = "pi%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },
}
