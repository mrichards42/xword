-- ============================================================================
-- messages.lua
--     Standard XWord message boxes
-- ============================================================================

if xword.logerror then
    local logerror = xword.logerror
    function xword.logerror(msg, ...)
        logerror(string.format(msg, ...))
    end
end

if wx then
    local function fmt(msg, ...)
        if select('#', ...) > 0 then
            return msg:format(...)
        else
            return msg
        end
    end
    function xword.Message(message, ...)
        wx.wxMessageBox(
            fmt(message, ...),
            "XWord Message",
            wx.wxOK + wx.wxICON_INFORMATION
        )
    end

    function xword.Prompt(message, ...)
        return wx.wxMessageBox(
            fmt(message, ...),
            "XWord Message",
            wx.wxYES_NO + wx.wxICON_QUESTION
        ) == wx.wxYES
    end

    function xword.Error(message, ...)
        wx.wxMessageBox(
            fmt(message, ...),
            "XWord Error",
            wx.wxOK + wx.wxICON_ERROR
        )
    end
end

if xword.frame then
    -- Check to see if a puzzle is loaded and optionally alert the user if no
    -- puzzle is loaded (the default)
    function xword.HasPuzzle(show_message)
        if not xword.frame.Puzzle:IsOk() then
            if show_message ~= false then
                xword.Message("No puzzle is opened");
            end
            return false
        end
        return true
    end
end