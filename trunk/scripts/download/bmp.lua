-- The wxBitmaps in the images folder

download.bmp = {}

local scriptsdir = xword.GetScriptsDir()

for _, name in pairs({
    'download',
    'play',
    'swap',
    'downloaderror',
}) do
    download.bmp[name] =
        wx.wxBitmap(wx.wxImage(
                    scriptsdir .. '/download/images/'..name..'.png',
                    wx.wxBITMAP_TYPE_PNG)
        )
end
