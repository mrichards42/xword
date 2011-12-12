require 'luacurl'

function download.get_default_puzzles()
return {
    {
        name = "NY Times Premium",
        url = "?",
        directoryname = "NY_Times",
        filename = "nyt%Y%m%d.puz",
        days = { true, true, true, true, true, true, true },
        curlopts =
        {},
        func = "return download.download(puzzle.url, puzzle.filename, puzzle.curlopts)",
        fields = { "User Name", "Password", "other field" }
    },

    {
        name = "NY Times (XWord Info)",
        url = "http://www.xwordinfo.com/XPF/?date=%m/%d/%Y",
        directoryname = "NY_Times",
        filename = "nyt%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://www.xwordinfo.com/',
        },
        func = "return download.download(puzzle.url, puzzle.filename, puzzle.curlopts)"
    },

    {
        name = "CrosSynergy",
        url = "http://www.washingtonpost.com/r/WashingtonPost/Content/Puzzles/Daily/cs%y%m%d.jpz",
        filename = "cs%Y%m%d.jpz",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Newsday",
        url = "http://picayune.uclick.com/comics/crnet/data/crnet%y%m%d-data.xml",
        filename = "nd%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "LA Times",
        url = "http://picayune.uclick.com/comics/tmcal/data/tmcal%y%m%d-data.xml",
        filename = "lat%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        filename = "usa%Y%m%d.xml",
        days = { true, true, true, true, true, false, false },
    },
    {
        name = "Ink Well",
        url = "http://herbach.dnsalias.com/Tausig/vv%y%m%d.puz",
        filename = "tausig%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "The Onion AV Club",
        url = "http://herbach.dnsalias.com/Tausig/av%y%m%d.puz",
        filename = "av%Y%m%d.puz",
        days = { false, false, true, false, false, false, false },
    },

    {
        name = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        filename = "jones%Y%m%d.puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        name = "Wall Street Journal",
        url = "http://mazerlm.home.comcast.net/wsj%y%m%d.puz",
        filename = "wsj%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Boston Globe",
        url = "http://home.comcast.net/~nshack/Puzzles/bg%y%m%d.puz",
        filename = "bg%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        name = "Philadelphia Inquirer",
        url = "http://mazerlm.home.comcast.net/pi%y%m%d.puz",
        filename = "pi%Y%m%d.puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        name = "The Chronicle of Higher Education",
        url = "http://chronicle.com/items/biz/puzzles/%Y%m%d.puz",
        filename = "che%Y%m%d.puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Universal",
        url = "http://picayune.uclick.com/comics/fcx/data/fcx%y%m%d-data.xml",
        filename = "univ%Y%m%d.xml",
        days = { true, true, true, true, true, true, true },
    },

}
end -- function get_default_puzzles

download.puzzles = download.get_default_puzzles()

-- Update the download sources
require 'download.config'
