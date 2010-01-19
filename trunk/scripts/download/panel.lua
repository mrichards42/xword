-- ============================================================================
-- A single download panel
--     A download is implemented as a table with the following keys:
--       panel       the wxPanel object
--       url         the download url
--       filename    the local filename
--       task_id     the id for the download thread (nil until the dl starts)
--       callbacks   a table of callback functions to respond to various
--                   messages sent by the download thread
--
--    download.newDownload() creates a new download panel and returns a table
--    with the aforementioned keys.  Pass the function a parent wxWindow,
--    a url, a filename, and an optional descriptive file type.  The file type
--    should be a type that the (separate) import add-on recognizes.  Omit the
--    file type if the file is a standard Across Lite .puz or .txt file.
-- ============================================================================

require 'mtask'
require 'download.defs'

-- Declare globals used by this module
local wx = wx
local task = task
local assert = assert
local unpack = unpack
local print = print
local xword = xword

-- ----------------------------------------------------------------------------
-- Utility functions
-- ----------------------------------------------------------------------------

-- Format bytes as a "human readable" string
local function formatSize(num)
    local abbrs = { 'bytes', 'kb', 'mb', 'gb' }
    for i=1,#abbrs do
        if num < 1024 then
            return string.format('%.1f %s', num, abbrs[i])
        end
        num = num / 1024
    end
    return string.format('%.1f %s', num, abbrs[#abbrs])
end


-- If there is no filename given, find a filename in the url.
-- If there is no filename in the url (i.e. http://www.example.com/),
-- use "index.html"
local function getFilename(url, filename)
    return filename or url:match('^.*/([^/]+)$') or 'index.html'
end

-- Make all directories in the path (filename must include a valid path)
local function makeDirs(filename)
    local fn = wx.wxFileName(filename)
    fn:Mkdir(4095, -- default permissions
             wx.wxPATH_MKDIR_FULL) -- Make intermediate dirs
end

-- Return the puzzle conversion function based on extension
local function getConversion(filename, type)
    if type then
        -- Two step assignment in case import.handlers[type] doesn't exist
        local data = import.handlers[type]
        if data then return data.convert end
    end

    local filename = filename or ''
    local file_ext = filename:match('^.*%.([%a%d]+)$')

    -- If we don't know the extension, or of XWord handles this file type
    -- natively (puz and txt).  No conversion necessary
    if not file_ext or file_ext == 'puz' or file_ext == 'txt' then
        return function() return true end
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
    return function() return true end
end

-- The wxBitmaps to use for buttons
local bmp = {
    download = wx.wxBitmap(wx.wxImage(
                    xword.scriptsdir .. '/download/images/download.png',
                    wx.wxBITMAP_TYPE_PNG)
    ),
    play = wx.wxBitmap(wx.wxImage(
                    xword.scriptsdir .. '/download/images/play.png',
                    wx.wxBITMAP_TYPE_PNG)
    ),
}


-- ===========================================================================
-- Everything declared after this statement is part of the download package
-- ===========================================================================
setfenv(1, assert(download, 'Must load download package first'))

-- ----------------------------------------------------------------------------
-- The download panel
-- ----------------------------------------------------------------------------

function newDownload(parent, url, filename, type)
    local filename = getFilename(url, filename)
    assert(parent and url and filename)

    local dl = {}
    dl.url = url
    dl.filename = filename
    dl.convert = getConversion(filename, type) -- Find the conversion function

    -- Call to start the download in a new thread
    local startDownloadLane = function()
        -- Ensure the download folder exists
        makeDirs(filename)
        dl.task_id = task.create(xword.scriptsdir..'download/task.lua', {1, url, filename})
    end

    -- Controls
    dl.panel = wx.wxPanel(
        parent, wx.wxID_ANY,
        wx.wxDefaultPosition, wx.wxDefaultSize,
        wx.wxTAB_TRAVERSAL + wx.wxBORDER_STATIC
    )
    dl.downloadButton = wx.wxBitmapButton(dl.panel, wx.wxID_ANY, bmp.download)
    dl.playButton = wx.wxBitmapButton(dl.panel, wx.wxID_ANY, bmp.play)
    dl.gauge = wx.wxGauge(
        dl.panel, wx.wxID_ANY, 0,
        wx.wxDefaultPosition, wx.wxSize(-1, 15)
    )
    dl.statusCtrl = wx.wxStaticText(dl.panel, wx.wxID_ANY, 'Connecting...')

    -- Layout
    dl.sizer = wx.wxBoxSizer(wx.wxVERTICAL)
        dl.buttonSizer = wx.wxBoxSizer(wx.wxHORIZONTAL)
            dl.buttonSizer:Add(dl.downloadButton, 0, wx.wxEXPAND + wx.wxALL, 5)
            dl.buttonSizer:Add(dl.playButton, 0, wx.wxEXPAND + wx.wxALL, 5)
        dl.sizer:Add(dl.buttonSizer, 0, wx.wxEXPAND + wx.wxALL, 5)
        dl.sizer:Add(dl.gauge, 0, wx.wxEXPAND + wx.wxALL, 5)
        dl.sizer:Add(dl.statusCtrl, 0, wx.wxEXPAND + wx.wxALL, 5)
        -- download info is hidden until the download actually starts
        dl.sizer:Show(dl.gauge, false)
        dl.sizer:Show(dl.statusCtrl, false)
    dl.panel:SetSizerAndFit(dl.sizer)

    -- Thread message callback functions
    dl.callbacks =
    {
        [DL_START] = function()
            -- do nothing
        end,

        [DL_PROGRESS] = function(args)
            print(args)
            dl.dlnow, dl.dltotal = unpack(args)

            if dl.dltotal > 0 then -- We know the content-length
                dl.gauge:SetRange(dl.dltotal)
                dl.gauge:SetValue(dl.dlnow)
                dl.gauge:SetToolTip(formatSize(dl.dlnow)..' / '..formatSize(dl.dltotal))
            else -- Unknown download size
                dl.gauge:Pulse()
                dl.gauge:SetToolTip(formatSize(dl.dlnow)..' / (unknown length)')
            end
        end,

        [DL_END] = function(args)
            -- Cleanup the download
            dl.res, dl.err = unpack(args)

            -- Convert the file if necessary
            if dl.res == 0 then
                if not dl.convert(filename, true) then -- Delete old file
                    dl.res = -1
                    dl.err = 'Could not open file'
                end
            end

            if dl.res == 0 then -- Successful download
                -- Indicate that the download is complete
                dl.gauge:SetRange(100)
                dl.gauge:SetValue(100)
                dl.statusCtrl:SetLabel('Done ('..formatSize(dl.dlnow)..')')
            else
                dl.statusCtrl:SetLabel('Error: '..dl.err)
                dl.statusCtrl:SetToolTip('Error: '..dl.err)
            end
            -- Display the status instead of the gauge
            dl.sizer:Show(dl.statusCtrl, true)
            dl.sizer:Show(dl.gauge, false)
            dl.sizer:Layout()
            -- Stop checking the message queue
            dl.panel:Disconnect(wx.wxEVT_IDLE)
        end,

        [DL_MESSAGE] = function(msg)
            print('Message: '..msg)
        end,

        -- Unknown messages get both the code and the data
        [DL_UNKNOWN] = function(code, data)
            print('Unknown message: '..code..':'..data)
        end,
    }

    -- Add a callback to the callbacks list; the old callback is called first.
    local function appendCallback(id, func)
        local oldCallback = dl.callbacks[id]
        dl.callbacks[id] = function(args)
            oldCallback(args)
            func(args)
        end
    end

    -- Add a callback to the callbacks list; the old callback is called last.
    local function prependCallback(id, func)
        local oldCallback = dl.callbacks[id]
        dl.callbacks[id] = function(args)
            func(args)
            oldCallback(args)
        end
    end


    -- The idle event : Check the download thread message queue
    local onIdle = function(evt)
        -- Check the message queue
        local data, flag, rc = task.receive(0, dl.task_id)
        if rc ~= 0 then return end

        -- find the callback function
        local callback = dl.callbacks[flag]
        if callback then
            callback(data)
        else
            dl.callbacks[download.DL_UNKNOWN](flag, data)
        end
    end

    local function startDownload()
        -- Show download info / hide button
        dl.sizer:Show(dl.gauge, true)
        dl.sizer:Show(dl.buttonSizer, false)
        dl.sizer:Layout()

        -- Start handling idle events (for download callbacks)
        dl.panel:Connect(wx.wxEVT_IDLE, onIdle)

        -- Start the download
        startDownloadLane()
    end

    -- Download button event handler
    dl.downloadButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function(evt)
            startDownload()
        end
    )

    -- Play button event handler
    dl.playButton:Connect(wx.wxEVT_COMMAND_BUTTON_CLICKED,
        function(evt)
            -- Open the file if the download completes successfully
            appendCallback(download.DL_END, function()
                if dl.res == 0 then
                    -- Make sure the file is a .puz (conversion happens in the
                    -- default DL_END callback).
                    local puzfile =
                        (dl.filename:match("^(.*)%..*$") or dl.filename)
                        ..'.puz'
                    xword.frame:LoadPuzzle(puzfile)
                end
            end)
            startDownload()
        end
    )

    return dl
end
