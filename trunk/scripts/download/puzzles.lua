download_dir = xword.userdatadir.."/puzzles"
download_fmt = '%Y%m%d'
require 'luacurl'

puzzles = 
{
    {
        name = "NY Times",
        url = "http://www.xwordinfo.com/XPF/?date=%m/%d/%Y",
        prefix = "nyt",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
        curlopts =
        {
            [curl.OPT_REFERER] = 'http://www.xwordinfo.com/',
        },
    },

    {
        name = "CrosSynergy",
        url = "http://www.washingtonpost.com/r/WashingtonPost/Content/Puzzles/Daily/cs%y%m%d.jpz",
        prefix = "cs",
        ext = "jpz",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "Newsday",
        url = "http://picayune.uclick.com/comics/crnet/data/crnet%y%m%d-data.xml",
        prefix = "nd",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "LA Times",
        url = "http://picayune.uclick.com/comics/tmcal/data/tmcal%y%m%d-data.xml",
        prefix = "lat",
        ext = "xml",
        days = { true, true, true, true, true, true, true },
    },

    {
        name = "USA Today",
        url = "http://picayune.uclick.com/comics/usaon/data/usaon%y%m%d-data.xml",
        prefix = "usa",
        ext = "xml",
        days = { true, true, true, true, true, false, false },
    },
    {
        name = "Ink Well",
        url = "http://herbach.dnsalias.com/Tausig/vv%y%m%d.puz",
        prefix = "tausig",
        ext = "puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "The Onion AV Club",
        url = "http://herbach.dnsalias.com/Tausig/av%y%m%d.puz",
        prefix = "av",
        ext = "puz",
        days = { false, false, true, false, false, false, false },
    },

    {
        name = "Jonesin'",
        url = "http://herbach.dnsalias.com/Jonesin/jz%y%m%d.puz",
        prefix = "jones",
        ext = "puz",
        days = { false, false, false, true, false, false, false },
    },

    {
        name = "Wall Street Journal",
        url = "http://mazerlm.home.comcast.net/wsj%y%m%d.puz",
        prefix = "wsj",
        ext = "puz",
        days = { false, false, false, false, true, false, false },
    },

    {
        name = "Boston Globe",
        url = "http://home.comcast.net/~nshack/Puzzles/bg%y%m%d.puz",
        prefix = "bg",
        ext = "puz",
        days = { false, false, false, false, false, false, true },
    },

    {
        name = "Philidelphia Inquirer",
        url = "http://mazerlm.home.comcast.net/pi%y%m%d.puz",
        prefix = "pi",
        ext = "puz",
        days = { false, false, false, false, false, false, true },
    },
}