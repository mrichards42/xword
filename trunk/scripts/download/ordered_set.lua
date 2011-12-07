-- An ordered set supporting append and prepend
-- Set an optional hash function to deal with equality issues

local mt = {}
mt.__index = mt

    function mt:remove(...)
        -- Make a set of the values
        local set = {}
        local arg = {n=select('#',...),...}
        local count = arg.n
        for i=1,arg.n do
            set[self.hash(arg[i])] = true
        end
        for i=#self,1,-1 do
            local v = self.hash(self[i])
            if set[v] then
                set[v] = nil
                table.remove(self, i)
                count = count - 1
                if count == 0 then
                    return
                end
            end
        end
    end

    function mt:clear()
        for i, _ in ipairs(self) do self[i] = nil end
    end

    function mt:append(...)
        self:remove(...)
        local arg = {n=select('#',...),...}
        for i=1,arg.n do
            table.insert(self, arg[i])
        end
    end

    function mt:insert(pos, ...)
        self:remove(...)
        local arg = {n=select('#',...),...}
        for i=arg.n,1,-1 do
            table.insert(self, pos, arg[i])
        end
    end

    function mt:prepend(...)
        self:insert(1, ...)
    end

    function mt:iter()
        local i = 0
        local n = #self
        return function ()
            i = i + 1
            if i <= n then return self[i] end
        end
    end

function OrderedSet(hash)
    local t = {}
    t.hash = hash or function(a) return a end
    setmetatable(t, mt)
    return t
end

return OrderedSet
