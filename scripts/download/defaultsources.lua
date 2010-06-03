-- Default download sources

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
    curlopts (optional)
        <table> a table of curl options for the download:
        { [curl.OPT_OPTION_NAME] = 'option value' }
        See luacurl documentation for valid option values
        
]]

require 'luacurl'

if not download then download = {} end

function download.getBasename(source, date)
    return source.prefix .. date:fmt(download.puzformat)
end

function download.getFilename(source, date)
    return download.localfolder .. '/' ..
           download.getBasename(source, date) .. '.' .. source.ext
end

function download.getPuzFile(source, date)
    return download.localfolder .. '/' ..
           download.getBasename(source, date) .. '.' .. 'puz'
end

download.sources = 
{
    {
        display = "NY Times",
        url = "http://www.xwordinfo.com/XPF/?date=%m/%d/%Y",
        prefix = "nyt",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
        format = "XPF",
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://www.xwordinfo.com/',
        },
    },

    {
        display = "CrosSynergy",
        url = "http://www.chron.com/apps/games/xword/puzzles/cs%y%m%d.puz",
        prefix = "cs",
        ext = "puz",
        days = { true, true, true, true, true, true, true },
    },

    {
        display = "Newsday",
        url = "http://picayune.uclick.com/comics/crnet/data/crnet%y%m%d-data.xml",
        prefix = "nd",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "LA Times",
        url = "http://picayune.uclick.com/comics/tmcal/data/tmcal%y%m%d-data.xml",
        prefix = "lat",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        prefix = "usa",
        ext = "xml",
        days = { true, true, true, true, true, false, false },
        format = 'XML (uclick)',
    },

    {
        display = "Universal",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        prefix = "univ",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
        format = 'XML (uclick)',
    },

    {
        display = "Ink Well",
        url = "http://herbach.dnsalias.com/Tausig/vv%y%m%d.puz",
        prefix = "tausig",
        ext = "puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        display = "The Onion AV Club",
        url = "http://herbach.dnsalias.com/Tausig/av%y%m%d.puz",
        prefix = "av",
        ext = "puz",
        days = { false, false, true, false, false, false, false },
    },

    {
        display = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        prefix = "jones",
        ext = "puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        display = "Wall Street Journal",
        url = "http://mazerlm.home.comcast.net/wsj%y%m%d.puz",
        prefix = "wsj",
        ext = "puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        display = "Boston Globe",
        url = "http://home.comcast.net/~nshack/Puzzles/bg%y%m%d.puz",
        prefix = "bg",
        ext = "puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        display = "Philidelphia Inquirer",
        url = "http://mazerlm.home.comcast.net/pi%y%m%d.puz",
        prefix = "pi",
        ext = "puz",
        days = { false, false, false, false, false, false, true },
    },
}
