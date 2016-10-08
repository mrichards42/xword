local DATA = {} -- win={[animation data]}
local STEP = 15 -- Step size in ms

local AnimateMove

local function end_animate(win)
    local data = DATA[win]
    if data and data.timer then
        data.timer:Stop()
        data.timer = nil
    end
    DATA[win] = nil
end

--- AnimateMove
AnimateMove = function(win, x, y, w, h, timeout)
    -- Setup animation variable
    local data = DATA[win] or {}
    DATA[win] = data
    local pos = win:GetPosition()
    local size = win:GetSize()
    data.x, data.y, data.w, data.h = pos.X, pos.Y, size.Width, size.Height
    -- If we aren't going to move, stop now
    if data.x == x and data.y == y and data.w == w and data.h == h then
        return end_animate(win)
    end
    -- Calculate movement per step
    data.timeout = timeout or 200
    data.steps = math.ceil(data.timeout / STEP)
    data.dx = (x - data.x)/data.steps
    data.dy = (y - data.y)/data.steps
    data.dw = (w - data.w)/data.steps
    data.dh = (h - data.h)/data.steps
    -- If the timer already exists the animation is already running
    if data.timer then return end
    -- Setup the timer
    data.timer = wx.wxTimer(win)
    data.timer:SetOwner(data.timer)
    data.timer:Connect(wx.wxEVT_TIMER, function(evt)
        data.steps = data.steps - 1
        if data.steps < 0 or not DATA[win] then
            return end_animate(win)
        end
        data.x = data.x + data.dx
        data.y = data.y + data.dy
        data.w = data.w + data.dw
        data.h = data.h + data.dh
        win:SetSize(data.x + .5, data.y + .5, data.w + .5, data.h + .5)
    end)
    -- Notify when win is destroyed
    win:Connect(win:GetId(), wx.wxEVT_DESTROY, function(evt)
        end_animate(win)
        evt:Skip()
    end)
    data.timer:Start(STEP)
end

return AnimateMove