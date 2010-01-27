-- The wxBitmaps in the images folder

download.bmp = {}

for _, name in pairs({
    'download',
    'play',
    'swap'
}) do
    download.bmp[name] =
        wx.wxBitmap(wx.wxImage(
                    xword.scriptsdir .. '/download/images/'..name..'.png',
                    wx.wxBITMAP_TYPE_PNG)
        )
end
