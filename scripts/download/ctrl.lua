-- ============================================================================
-- A single download control
--     A download is implemented as a table with the following keys:
--       ctrl        the wxPanel object
--       url         the download url
--       filename    the local filename
--       basename    the basename (without path or extension)
--       puzfile     the local filename after conversion to .puz
--       task_id     the id for the download thread (nil until the download
--                   starts)
--       callbacks   a table of callback functions to respond to messages
--                   sent by the download thread (messages defined in download.defs)
--       res / err   the download result and error message from cURL (nil until
--                   the download ends)
--       start()     start the download
--       isrunning() is the download currently running? (implies that task_id
--                   is not nil)
--
--    download.DownloadCtrl() creates a new download panel and returns a table
--    with the aforementioned keys.  Pass the function a parent wxWindow,
--    a url, a filename, and an optional descriptive file type.  The file type
--    should be a type that the (separate) import add-on recognizes.  Omit the
--    file type if the file is a standard Across Lite .puz or .txt file.
-- ============================================================================

require 'mtask'
require 'download.defs'
require 'lfs'
require 'date'
require 'download.bmp'

-- Declare globals used by this module
local wx = wx
local task = task
local assert = assert
local unpack = unpack
local print = print
local xword = xword
local lfs = lfs
local os = os
local date = date
local bmp = download.bmp
local type = type

-- ----------------------------------------------------------------------------
-- Utility functions
-- ----------------------------------------------------------------------------


-- If there is no filename given, find a filename in the url.
-- If there is no filename in the url (i.e. http://www.example.com/),
-- use "index.html"
local function getFilename(url, filename)
    return filename or url:match('^.*/([^/]+)$') or 'index.html'
end


-- Try to load the puzzle. if it doesn't work, return false
-- This is the "conversion" for .puz and .txt files
local function checkPuz(filename)
    local success, ret = pcall(xword.XPuzzle, filename)
    if success then
        ret = nil -- Garbage collect the puzzle
        return true
    end
    -- error from XPuzzle constructor: { "messsage", "class", fatal (bool) }
    return false, ret[1]
end


-- Return the puzzle conversion function based on extension
local function getConversion(filename, loadtype)
    if loadtype then
        -- Two step assignment in case import.handlers[loadtype] doesn't exist
        local data = import.handlers[loadtype]
        if data then return data.convert end
    end

    local filename = filename or ''
    local file_ext = filename:match('^.*%.([%a%d]+)$')

    -- If we don't know the extension, or if XWord handles this file type
    -- natively (puz and txt), just try to open the file without conversion
    if not file_ext or file_ext == 'puz' or file_ext == 'txt' then
        return checkPuz
    end

    -- Find the extension in the import handlers
    for label, data in pairs(import.handlers) do
        for ext, _ in pairs(data.types) do
            if ext == file_ext then
                return data.convert
            end
        end
    end

    -- Fallback to no conversion
    return checkPuz
end


local P = download

-- ----------------------------------------------------------------------------
-- The download control
-- ----------------------------------------------------------------------------
--[[
Create a download control.
Return nil if there is no valid download.

Parameters:
    DownloadCtrl(parent, source, date):
        source: a table from the download.sources table.
        date:   the date for the download.

    DownloadCtrl(parent, url, [curlopts], [filename], [loadtype]):
        url:      the url to download
        filename: the output filename.  If not supplied, use the filename
                  from the url.
        curlopts: a table of curl opts:  { [curl.OPT_NAME] = 'value', }
        loadtype: the file type of the downloaded puzzle, used to convert
                  the download to a valid .puz file.  If not supplied, guess
                  based on the extension of the url.  Default to Across
                  .puz or .txt formats.
]]

function P.DownloadCtrl(parent, ...)
    local dl = {}

    -- ------------------------------------------------------------------------
    -- Arguments
    -- ------------------------------------------------------------------------
    -- DownloadCtrl(parent, source, date)
    if #arg == 2 and type(arg[1]) == 'table' then
        local source, d = unpack(arg)
        if not source.days[d:getisoweekday()] then return nil end
        dl.url = d:fmt(source.url)
        dl.filename = P.getFilename(source, d)
        dl.convert = getConversion(dl.filename, source.format)
        dl.curlopts = source.curlopts

    -- DownloadCtrl(parent, url, [filename], [loadtype], [curlopts])
    else
        dl.url, dl.filename, type, dl.curlopts = unpack(arg)
        dl.filename = getFilename(dl.url, dl.filename)
        dl.convert = getConversion(dl.filename, type)
    end

    assert(dl.url and dl.filename and dl.convert)

    -- The name of the file after it has been converted to .puz
    dl.basename = dl.filename:match(".*/([^/%.]+)%..*")
    dl.puzfile = (dl.filename:match("^(.*)%..*$") or dl.filename)..'.puz'

    -- ------------------------------------------------------------------------
    -- Controls
    -- ------------------------------------------------------------------------
    dl.ctrl = wx.wxPanel(
        parent, wx.wxID_ANY,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTAB_TRAVERSAL + wx.wxBORDER_DOUBLE,
        "DownloadCtrl"
    )
    local downloadButton = wx.wxBitmapButton(
        dl.ctrl, wx.wxID_ANY, bmp.download,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxBORDER_NONE
    )
    downloadButton:SetToolTip("Download")
    local playButton = wx.wxBitmapButton(
        dl.ctrl, wx.wxID_ANY, bmp.play,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxBORDER_NONE
    )
    playButton:SetToolTip("Open")
    local gauge = wx.wxGauge(
        dl.ctrl, wx.wxID_ANY, 0,
        wx.wxDefaultPosition, wx.wxSize(45, 16)
    )
    local statusCtrl = wx.wxStaticText(dl.ctrl, wx.wxID_ANY, 'Status')

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------

    local sizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
        sizer:Add(gauge, 1, wx.wxALL + wx.wxALIGN_CENTER_VERTICAL, 5)
        sizer:Add(statusCtrl, 0, wx.wxALL + wx.wxALIGN_CENTER_VERTICAL, 5)
        local buttonSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            buttonSizer:Add(downloadButton, 0, wx.wxALL, 5)
            buttonSizer:Add(playButton, 0, wx.wxALL, 5)
        sizer:Add(buttonSizer, 0, wx.wxEXPAND + wx.wxALL + wx.wxALIGN_CENTER_VERTICAL, 0)
    -- Keep everything centered horizontally
    local bordersizer = wx.wxBoxSizer(wx.wxVERTICAL)
    bordersizer:Add(sizer, 1, wx.wxEXPAND + wx.wxALIGN_CENTER, 0)
    dl.ctrl:SetSizer(bordersizer)

    -- Change layout based on status of puzfile
    local function UpdateLayout()
        -- As a rule, show, hide, and layout top level sizers first, then
        -- mess with nested sizers

        -- The download has completed
        if dl.res then
            sizer:Show(buttonSizer, true)
            sizer:Show(statusCtrl, false)
            sizer:Show(gauge, false)
            sizer:Layout()
            if dl.res == 0 then -- Successful download
                print('Download successful:'..dl.filename)
                buttonSizer:Show(playButton, true)
                buttonSizer:Show(downloadButton, false)
                buttonSizer:Layout()
                print("Is download button shown?"..tostring(buttonSizer:GetItem(downloadButton):IsShown()))
            else -- Unsuccessful download
                downloadButton:SetToolTip('Error: '..dl.err..'  Redownload.')
                downloadButton:SetBitmapLabel(bmp.downloaderror)
                buttonSizer:Show(playButton, true)
                buttonSizer:Show(downloadButton, true)
                buttonSizer:Layout()
            end

        -- The download has not started
        elseif not dl.isrunning() then
            local stats = download.sourcecache[dl.basename]
            if not stats then
                print('Trying to cache: '..dl.puzfile)
                stats = download.cachePuzzle(dl.puzfile)
            end
            if not stats then -- Puzzle does not exist
                sizer:Show(gauge, false)
                sizer:Show(statusCtrl, false)
                sizer:Show(buttonSizer, true)
                sizer:Layout()
            else -- Puzzle exists
                dl.fileexists = true
                sizer:Show(gauge, false)
                sizer:Show(buttonSizer, true)
                if stats.started then -- Display puzzle stats if it has been opened
                    sizer:Show(statusCtrl, true)
                    local status = stats.complete.."%"
                     -- Display time if it is > 0
                    if stats.time > 0 then
                        if stats.time > 60*60 then
                            status = status.."\n"..date(stats.time):fmt("%H:%M:%S")
                        else
                            status = status.."\n"..date(stats.time):fmt("%M:%S")
                        end
                    end
                    statusCtrl:SetLabel(status)
                else
                    sizer:Show(statusCtrl, false)
                end
                buttonSizer:Show(downloadButton, false)
                buttonSizer:Show(playButton, true)
                buttonSizer:Layout()
            end
        else -- Download has just started
            sizer:Show(gauge, true)
            sizer:Show(buttonSizer, false)
            sizer:Show(statusCtrl, false)
        end

        sizer:Layout()
        parent:Layout()
    end

    -- ------------------------------------------------------------------------
    -- Thread message callbacks
    -- ------------------------------------------------------------------------
    local dlnow, dltotal = 0, 0

    dl.callbacks =
    {
        [P.DL_START] = function()
            -- Nothing to see here
        end,

        [P.DL_PROGRESS] = function(args)
            dlnow, dltotal = unpack(args)

            if dltotal > 0 then -- We know the content-length
                gauge:SetRange(dltotal)
                gauge:SetValue(dlnow)
            else -- Unknown download size
                gauge:Pulse()
            end
        end,

        [P.DL_END] = function(args)
            -- Stop checking the message queue
            dl.ctrl:Disconnect(wx.wxEVT_IDLE)
            dl.task_id = nil

            -- Cleanup the download
            dl.res, dl.err = unpack(args)

            -- Convert the file if necessary
            if dl.res == 0 then
                local success, err = dl.convert(dl.filename, true) -- Delete old file
                if not success then
                    dl.res = -1
                    dl.err = err or "Unknown"
                    -- If we can't open the file, delete it.
                    os.remove(dl.filename)
                    dl.fileexists = false
                elseif dl.openOnSuccess then
                    xword.frame:LoadPuzzle(dl.puzfile)
                    dl.fileexists = true
                end
            else
                -- If the download didn't work, delete the file.
                os.remove(dl.filename)
                dl.fileexists = false
            end

            dl.openOnSuccess = nil

            P.dlg.NotifyDownloadEnd(dl)

            UpdateLayout()
        end,

        [P.DL_MESSAGE] = function(msg)
            print('Message: '..msg)
        end,

        -- Unknown messages get both the code and the data
        [P.DL_UNKNOWN] = function(code, data)
            print('Unknown message: '..code..':'..data)
        end,
    }

    -- ------------------------------------------------------------------------
    -- Event Handlers
    -- ------------------------------------------------------------------------

    -- The idle event:
    -- Check the download thread message queue and call the appropriate
    -- message callbacks
    local function onIdle(evt)
        assert(dl.isrunning())

        -- Check the message queue
        local data, flag, rc = task.receive(0, dl.task_id)
        if rc ~= 0 then return end

        -- Find the callback function and call it
        local callback = dl.callbacks[flag]
        if callback then
            callback(data)
        else
            dl.callbacks[P.DL_UNKNOWN](flag, data)
        end
    end


    -- Download button event handler
    downloadButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function(evt)
            dl.start()
        end
    )

    -- Play button event handler
    playButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function(evt)
            -- Try to open the file before downloading
            if lfs.attributes(dl.puzfile) then -- Does the file exist?
                xword.frame:LoadPuzzle(dl.puzfile)
            else
                dl.openOnSuccess = true
                dl.start()
            end
        end
    )

    -- ------------------------------------------------------------------------
    -- Public functions
    -- ------------------------------------------------------------------------
    function dl.start()
        -- Don't re-start currently running downloads.
        if dl.isrunning() then print ('Already running: '..dl.filename) return end

        dl.res = nil
        dl.err = nil

        -- Start handling idle events (for download callbacks)
        dl.ctrl:Connect(wx.wxEVT_IDLE, onIdle)

        -- Ensure the download folder exists
        xword.makeDirs(dl.filename)

        -- Delete the output files
        os.remove(dl.filename)
        os.remove(dl.puzfile)

        -- Start the download in a new thread
        local args = { 1, dl.url, dl.filename }
        if dl.curlopts then
            for k,v in pairs(dl.curlopts) do
                table.insert(args, k)
                table.insert(args, v)
            end
        end
        dl.task_id = task.create(xword.GetScriptsDir()..'/download/task.lua',
                                 args)

        P.dlg.NotifyDownloadStart(dl)

        UpdateLayout()
    end

    function dl.isrunning()
        -- task_id will be set to nil when the download completes
        return dl.task_id
    end

    UpdateLayout()
    return dl
end
