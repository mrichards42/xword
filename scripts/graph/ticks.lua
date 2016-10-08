-- Return a "nice" number (powers of ten of 1,2,5)
-- nice_number(n, 'ceiling' | 'round' | 'flood' )
local function nice_number(n, est_method)
    method = method or 'ceiling'
    -- Get the order of magnitude
    local exponent = math.floor(math.log10(n))
    -- Get the first number
    local fraction = n / 10 ^ exponent
    -- Turn the first number into 1, 2, 5, or 10
    local nice
    if method == 'floor' then
        if fraction < 2 then
            nice = 1
        elseif fraction < 5 then
            nice = 2
        elseif fraction < 10 then
            nice = 5
        else
            nice = 10
        end
    elseif method == 'round' then
        if fraction < 1.5 then
            nice = 1
        elseif fraction < 3 then
            nice = 2
        elseif fraction < 7 then
            nice = 5
        else
            nice = 10
        end
    else -- Ceiling
        if fraction <= 1 then
            nice = 1
        elseif fraction <= 2 then
            nice = 2
        elseif fraction <= 5 then
            nice = 5
        else
            nice = 10
        end
    end
    -- Put us back into the right order of magnitude
    return nice * 10 ^ exponent
end

-- Return a list of tick marks given min, max, and max_ticks
local function get_tick_list(min, max, max_ticks)
    max_ticks = max_ticks or 10
    local range = nice_number(max - min, 'ceiling');
    local spacing = nice_number(range / (max_ticks - 1), 'floor')
    local min_tick = math.floor(min / spacing) * spacing
    local max_tick = math.floor(max / spacing) * spacing
    local ticks = {}
    for tick=min_tick,max_tick,spacing do
        table.insert(ticks, math.floor(tick / spacing) * spacing)
    end
    return ticks
end

-- Return a list of tick marks:
-- { { label = 'label', value = number }, ...}
-- fmt should be a function, nil, or 'percent'
local function get_ticks(min, max, max_ticks, fmt)
    if not fmt then
        fmt = function (n) return n end
    end
    -- Get just the tick mark values
    local ticks
    if is_percent then
        ticks = get_tick_list(min/max * 100, 100, max_ticks)
    else
        ticks = get_tick_list(min, max, max_ticks)
    end
    -- Set label and value fields
    local t = {}
    for _, tick in ipairs(ticks) do
        -- If these are percent marks add % to the label
        -- and find the location of the tick mark between min and max
        table.insert(t, {
            value = is_percent and (tick / 100 * max) or tick,
            label = tostring(is_percent and tick..'%' or fmt(tick)),
        })
    end
    return t
end

return get_ticks