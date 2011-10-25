-- ============================================================================
-- A single download control
--     A download is implemented as a table with the following keys:
--       ctrl        the wxPanel object
--       url         the download url
--       filename    the local filename
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
--    should be a type that the (separate) import add-on recognizes.
-- ============================================================================

require 'wxtask'
require 'download.messages'
require 'lfs'
require 'date'
require 'download.bmp'
local database = require 'download.database'
local makepath = require 'pl.dir'.makepath
local path = require 'pl.path'
local seq = require 'pl.seq'
local utils = require 'pl.utils'

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
local select = select
local min = math.min

local P = download

-- ----------------------------------------------------------------------------
-- Utility functions
-- ----------------------------------------------------------------------------

-- Copy a text to the clipboard
local function copy_text(text)
    local clipBoard = wx.wxClipboard.Get()
    if clipBoard and clipBoard:Open() then
        clipBoard:SetData(wx.wxTextDataObject(text))
        clipBoard:Flush() -- Make this available after we've exited
        clipBoard:Close()
    end
end

-- ----------------------------------------------------------------------------
-- A spinning download gauge
-- ----------------------------------------------------------------------------

P.onInit(function()
    P.downloadBmps = {
        spin = {},
    }
    setmetatable(P.downloadBmps,
        {
            __index = function(self, name)
                local bmp = P.bmp[name]
                rawset(self, name, bmp)
                return bmp
            end
        }
    )

    -- Extract the spinning images
    do
        local spin = P.downloadBmps.spin
        local working = P.img.working
        for y=0,63,16 do
            for x=0,127,16 do
                -- the first image is blank
                if not (x == 0 and y == 0) then
                    table.insert(spin, working:GetSubImage(wx.wxRect(x,y, 16, 16)))
                end
            end
        end
        -- Convert the images to bitmaps
        for i, img in ipairs(spin) do
            spin[i] = wx.wxBitmap(img)
            img:delete()
        end
        P.img.working = nil
    end
end) -- onInit

P.onClose(function()
    -- delete images
    for _, bmp in ipairs(P.downloadBmps.spin) do
        bmp:delete()
    end
    P.downloadBmps = nil
end) -- onClose

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
]]

function P.DownloadCtrl(parent, source, d)
    local dl = {}

    -- Do we have a download for this date?
    if not source.days[d:getisoweekday()] then return nil end

    dl.url = d:fmt(source.url) -- remote url
    dl.basename = P.get_basename(source, d)
    dl.filename = P.get_download_filename(source, d) -- filename to save
    dl.format = source.format
    dl.curlopts = source.curlopts

    assert(dl.url and dl.filename)

    -- Get stats for the puzzle
    dl.stats = database.cachePuzzle(dl.basename)
    if not dl.stats.filename then
        dl.stats = nil
    end
    dl.hasstats = true

    -- ------------------------------------------------------------------------
    -- The control
    -- ------------------------------------------------------------------------
    dl.ctrl = wx.wxControl(parent, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxSize(16,16), wx.wxBORDER_NONE)
    local ctrl = dl.ctrl
    local bmps = P.downloadBmps
    ctrl.bmp = bmps.status.download
    ctrl.bmpindex = 0

    --ctrl:SetMaxSize(wx.wxSize(16,16))
    ctrl:SetMinSize(wx.wxSize(16,16))

    function ctrl.Draw(dc)
        if ctrl.bmpindex > 0 then
            ctrl.bmp = bmps.spin[ctrl.bmpindex]
            dc:Clear()
        end
        dc:DrawBitmap(ctrl.bmp, 0, 0, true)
    end

    function ctrl.OnPaint(evt)
        local dc = wx.wxPaintDC(ctrl)
        ctrl.Draw(dc)
        dc:delete()
    end
    ctrl:Connect(wx.wxEVT_PAINT, ctrl.OnPaint)

    function ctrl.OnTimer()
        if ctrl.bmpindex > 0 then
            ctrl.bmpindex = ctrl.bmpindex + 1
            if ctrl.bmpindex > #bmps.spin then
                ctrl.bmpindex = 1
            end
            local dc = wx.wxClientDC(ctrl)
            ctrl.Draw(dc)
            dc:delete()
        end
    end

    -- Change the bitmap and tooltip based on status of the downloaded file
    function ctrl.UpdateCtrl()
        ctrl.bmpindex = 0 -- not downloading

        -- The stats have not yet been retrieved from the database
        -- For now, we're calling wxYield() after the creation of every
        -- downloadCtrl, so the length of time this takes shouldn't be
        -- an issue
        if not dl.hasstats then
            ctrl.bmp = bmps.status.unknown
            ctrl:SetToolTip('Querying...')

        -- The download has completed
        elseif dl.res then
            if dl.res == 0 then -- Successful download
                ctrl.bmp = bmps.status.exists
                ctrl:SetToolTip('Open')
                -- Pass
            else -- Unsuccessful download
                ctrl.bmp = bmps.status.error
                ctrl:SetToolTip('Error: '..dl.err)
            end

        -- The download has not started
        elseif not dl.isrunning() then
            local stats = dl.stats
            if not stats then -- Puzzle does not exist
                ctrl.bmp = bmps.status.download
                ctrl:SetToolTip('Download')
            else -- Puzzle exists
                dl.fileexists = true
                if stats.started then -- Display puzzle stats if it has been opened
                    local status = {}
                    if stats.complete == 100 then
                        ctrl.bmp = bmps.status.complete
                        table.insert(status, 'Complete')
                    else
                        ctrl.bmp = bmps.status.working
                        table.insert(status, stats.complete..'%')
                    end
                     -- Display time if it is > 0
                    if stats.time > 0 then
                        if stats.time > 60*60 then
                            table.insert(status, date(stats.time):fmt("%H:%M:%S"))
                        else
                            table.insert(status, date(stats.time):fmt("%M:%S"))
                        end
                    end
                    ctrl:SetToolTip(table.concat(status,'\n'))
                else
                    ctrl.bmp = bmps.status.exists
                    ctrl:SetToolTip('Open')
                end
            end

        -- Download has just started
        else
            ctrl:SetToolTip('Downloading...')
            ctrl.bmpindex = 1
        end
        ctrl:Refresh()
    end

    -- ------------------------------------------------------------------------
    -- Thread message callbacks
    -- ------------------------------------------------------------------------
    local dlnow, dltotal = 0, 0

    dl.callbacks =
    {
        [task.START] = function()
            P.dlg.NotifyDownloadStart(dl)
        end,

        [P.DL_PROGRESS] = function(args)
            dlnow, dltotal = unpack(args)
--[[
            if dltotal > 0 then -- We know the content-length
                gauge:SetRange(dltotal)
                gauge:SetValue(dlnow)
            else -- Unknown download size
                gauge:Pulse()
            end
]]
        end,

        [P.DL_END] = function(err)
            dl.task_id = nil
            dl.err = err
        end,

        [task.END] = function()
            if not dl.err then
                dl.res = 0
                if dl.openOnSuccess then
                    xword.frame:LoadPuzzle(dl.filename)
                else
                    -- Cache the puzzle
                    database.cacheInBackground(dl.basename)
                end
                dl.fileexists = true
            else
                dl.res = -1
                -- If the download didn't work, delete the file.
                os.remove(dl.filename)
            end

            dl.openOnSuccess = nil

            ctrl.UpdateCtrl()

            P.dlg.NotifyDownloadEnd(dl)
        end,
    }

    -- ------------------------------------------------------------------------
    -- Event Handlers
    -- ------------------------------------------------------------------------
    ctrl:Connect(wx.wxEVT_LEFT_DOWN,
        function(evt)
            ctrl:CaptureMouse()
        end
    )
    ctrl:Connect(wx.wxEVT_MOUSE_CAPTURE_LOST,
        function(evt)
           ctrl:ReleaseMouse()
        end
    )
    ctrl:Connect(wx.wxEVT_LEFT_UP,
        function(evt)
            if not ctrl:HasCapture() then
                return
            end
            ctrl:ReleaseMouse()

            if dl.isrunning() or
               not wx.wxRect(
                        wx.wxPoint(0,0), ctrl:GetSize()
                   ):Contains(evt:GetPosition())
            then
                return
            end

            -- Try to open the file before downloading
            if lfs.attributes(dl.filename) then
                if xword.frame:LoadPuzzle(dl.filename) then
                    return
                end
            end
            -- If we can't open the file, download it
            dl.openOnSuccess = true
            dl.start()
        end
    )

    -- Context menu:
        -- Copy URL;
        -- Copy local filename
    ctrl:Connect(wx.wxEVT_CONTEXT_MENU,
        function(evt)
            local menu = wx.wxMenu()
            local item
            item = menu:Append(wx.wxID_ANY, "Copy URL")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) copy_text(dl.url) end)
            item = menu:Append(wx.wxID_ANY, "Copy local filename")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) copy_text(dl.filename) end)
            item = menu:Append(wx.wxID_ANY, "Redownload")
            ctrl:Connect(item:GetId(),
                         wx.wxEVT_COMMAND_MENU_SELECTED,
                         function (evt) dl.start() end)
            ctrl:PopupMenu(menu)
            menu:delete()
        end
    )

    -- ------------------------------------------------------------------------
    -- Public functions
    -- ------------------------------------------------------------------------
    function dl.start()
        -- Don't re-start currently running downloads.
        if dl.isrunning() then return end

        dl.res = nil
        dl.err = nil

        -- Ensure the download folder exists
        makepath(path.dirname(dl.filename))

        -- Delete the output files
        os.remove(dl.filename)

        -- Start the download in a new thread      
        dl.task_id = task.create('download.task',
                                 { dl.url,
                                   dl.filename,
                                   dl.curlopts or {}, -- don't put a hole in this table
                                 })
        local e = task.handleEvents(dl.task_id, dl.callbacks, dl.ctrl)
        e.name = dl.filename

        ctrl.UpdateCtrl()
    end

    function dl.isrunning()
        -- task_id will be set to nil when the download completes
        return dl.task_id
    end

    function dl.has_ctrl()
        return dl.ctrl and getmetatable(dl.ctrl)
    end

    ctrl.UpdateCtrl()
    return dl
end
