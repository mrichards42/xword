local function init()
    -- Add a "debug" menu if it does not exist
    xword.frame:GetMenu('Debug')

    -- Add puz library functions
    require 'luapuz'

    -- Return a set of tables for the grid
    local grid_index = puz.Grid._index
    function puz.Grid:_index(name)
        if type(name) ~= "string" then
            return grid_index(self, name)
        end

        local func = nil
        -- Try to return a table of values

        -- Normalize the function name so that we can use it as a method
        -- on the squares.
        local isstring = false -- Return as a table or a string?
        local isrows = false

        func = puz.Square[name]

        if not func then
            -- GetXXXXXTable => GetXXXX
            if name:sub(-5) == "Table" then
                name = name:sub(1,-6)
                isstring = false
            -- GetXXXXXString => GetXXXX
            elseif name:sub(-6) == "String" then
                name = name:sub(1,-7)
                isstring = true
            -- GetXXXXXRows => GetXXXX
            elseif name:sub(-4) == "Rows" then
                name = name:sub(1,-5)
                isstring = true
                isrows = true
            end
            func = puz.Square[name]
        end

        -- XXXXX => GetXXXX
        if not func and name:sub(1,3) ~= "Get" and name:sub(1,3) ~= "Set" then
            name = "Get"..name
            func = puz.Square[name]
        end
        -- GetSquareXXXXX => GetXXXX
        if not func and name:sub(4,9) == "Square" then
            name = name:sub(1,3)..name:sub(10)
            func = puz.Square[name]
        end

        if not func then return nil end

        return function(...)
            -- Make a table for each row and fill it
            local t = {}
            for i = 1,self:LastRow() do
                t[i] = {}
                for j = 1,self:LastCol() do
                    local square = grid_index(self, {j, i})
                    t[i][j] = func(square, unpack(arg))
                end
            end
            if isstring then
                for i,it in ipairs(t) do
                    for j,v in ipairs(it) do
                        t[i][j] = tostring(v)
                    end
                    t[i] = table.concat(it)
                end
                if isrows then
                    return t
                else
                    return table.concat(t,'\n')
                end
            end
            return t
        end
    end

    p = xword.frame.Puzzle
    g = p.Grid

    require 'xworddebug.globals'
--    require 'xworddebug.test_files'
    require 'xworddebug.html'

    -- This is always last
    require 'xworddebug.interp'
end

return init
