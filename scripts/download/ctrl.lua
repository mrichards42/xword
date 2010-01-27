-- ============================================================================
-- A single download control
--     A download is implemented as a table with the following keys:
--       ctrl        the wxPanel object
--       url         the download url
--       filename    the local filename
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

    DownloadCtrl(parent, url, [filename], [loadtype]):
        url:      the url to download
        filename: the output filename.  If not supplied, use the filename
                  from the url.
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
        dl.filename = P.localfolder..'\\'..d:fmt(source.filename)
        dl.convert = getConversion(dl.filename, source.format)

    -- DownloadCtrl(parent, url, [filename], [loadtype])
    else
        dl.url, dl.filename, type = unpack(arg)
        dl.filename = getFilename(dl.url, dl.filename)
        dl.convert = getConversion(dl.filename, type)
    end

    assert(dl.url and dl.filename and dl.convert)

    -- The name of the file after it has been converted to .puz
    dl.puzfile = (dl.filename:match("^(.*)%..*$") or dl.filename)..'.puz'

    -- ------------------------------------------------------------------------
    -- Controls
    -- ------------------------------------------------------------------------
    dl.ctrl = wx.wxPanel(
        parent, wx.wxID_ANY,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTAB_TRAVERSAL + wx.wxBORDER_NONE,
        "DownloadCtrl"
    )
    local downloadButton = wx.wxBitmapButton(dl.ctrl, wx.wxID_ANY, bmp.download, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxBORDER_NONE)
    local playButton = wx.wxBitmapButton(dl.ctrl, wx.wxID_ANY, bmp.play, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxBORDER_NONE)
    local gauge = wx.wxGauge(
        dl.ctrl, wx.wxID_ANY, 0,
        wx.wxDefaultPosition, wx.wxSize(-1, 15)
    )
    local statusCtrl = wx.wxStaticText(dl.ctrl, wx.wxID_ANY, 'Connecting...')

    -- ------------------------------------------------------------------------
    -- Layout
    -- ------------------------------------------------------------------------
    local sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        local buttonSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            buttonSizer:Add(downloadButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            buttonSizer:Add(playButton, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(buttonSizer, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(gauge, 0, wx.wxEXPAND + wx.wxALL, 5)
        sizer:Add(statusCtrl, 0, wx.wxEXPAND + wx.wxALL, 5)
        -- download info is hidden until the download actually starts
        sizer:Show(gauge, false)
        sizer:Show(statusCtrl, false)
    dl.ctrl:SetSizerAndFit(sizer)

    -- ------------------------------------------------------------------------
    -- Thread message callbacks
    -- ------------------------------------------------------------------------
    local dlnow, dltotal = 0, 0

    dl.callbacks =
    {
        [P.DL_START] = function()
            -- do nothing
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
            -- Cleanup the download
            dl.res, dl.err = unpack(args)

            -- Convert the file if necessary
            if dl.res == 0 then
                local success, err = dl.convert(dl.filename, true) -- Delete old file
                if not success then
                    dl.res = -1
                    dl.err = err
                    -- If we can't open the file, delete it.
                    os.remove(dl.filename)
                end
            end

            if dl.res == 0 then -- Successful download
                -- Indicate that the download is complete
                gauge:SetRange(100)
                gauge:SetValue(100)
                statusCtrl:SetLabel('Done')
                print("(DL_END) Success: " .. dl.url)
            else
                if not dl.err then dl.err = 'Unknown' end
                statusCtrl:SetLabel('Error: '..dl.err)
                statusCtrl:SetToolTip('Error: '..dl.err)
                print("(DL_END) Error: " .. dl.url)
            end
            -- Display the status instead of the gauge
            sizer:Show(statusCtrl, true)
            sizer:Show(gauge, false)
            sizer:Layout()
            -- Stop checking the message queue
            dl.ctrl:Disconnect(wx.wxEVT_IDLE)
        end,

        [P.DL_MESSAGE] = function(msg)
            print('Message: '..msg)
        end,

        -- Unknown messages get both the code and the data
        [P.DL_UNKNOWN] = function(code, data)
            print('Unknown message: '..code..':'..data)
        end,
    }

    -- Add a callback to the callbacks list.  The old callback is called first.
    local function appendCallback(id, func)
        local oldCallback = dl.callbacks[id]
        dl.callbacks[id] = function(args)
            oldCallback(args)
            func(args)
        end
    end

    -- Add a callback to the callbacks list.  The old callback is called last.
    local function prependCallback(id, func)
        local oldCallback = dl.callbacks[id]
        dl.callbacks[id] = function(args)
            func(args)
            oldCallback(args)
        end
    end

    -- ------------------------------------------------------------------------
    -- Event Handlers
    -- ------------------------------------------------------------------------

    -- The idle event:
    -- Check the download thread message queue and call the appropriate
    -- message callbacks
    local function onIdle(evt)
        -- Check the message queue
        local data, flag, rc = task.receive(0, dl.task_id)
        if rc ~= 0 then return end

        -- find the callback function
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
                -- Open the file if the download completes successfully
                appendCallback(P.DL_END, function()
                    if dl.res == 0 then
                        -- Make sure the file is a .puz (conversion happens in the
                        -- default DL_END callback).
                        xword.frame:LoadPuzzle(dl.puzfile)
                    end
                end)
                dl.start()
            end
        end
    )

    -- ------------------------------------------------------------------------
    -- Public functions
    -- ------------------------------------------------------------------------
    function dl.start()
        -- Don't re-start currently running downloads.
        if dl.isrunning() then return end

        -- Show download info / hide button
        sizer:Show(gauge, true)
        sizer:Show(buttonSizer, false)
        sizer:Layout()

        -- Start handling idle events (for download callbacks)
        dl.ctrl:Connect(wx.wxEVT_IDLE, onIdle)

        -- Ensure the download folder exists
        xword.makeDirs(dl.filename)

        -- Start the download in a new thread
        dl.task_id = task.create(xword.scriptsdir..'/download/task.lua',
                                 {1, dl.url, dl.filename})
    end

    function dl.isrunning()
        return dl.task_id and task.isrunning(dl.task_id)
    end

    return dl
end
