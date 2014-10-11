--------------------------------------------------------------------------------
-- Serialize an object into a source code string. This string, when passed as
-- an argument to loadstring()(), returns an object structurally identical
-- to the original one. The following are currently supported:
--
-- * strings, numbers, booleans, nil
-- * functions without upvalues
-- * tables thereof. Tables can have shared part, but can't be recursive yet.
-- Caveat: metatables and environments aren't saved.

-- Additions for XWord:
-- serialize is made into a table where the metamethod __call is the same
-- as the original serialize function.
-- Serialize and load dates
--     serialize.pprint() is used for pretty-printing
--     serialize.dump() is used for output to a file
--     serialize.loadfile() safely loads a file
--     serialize.loadstring() safely loads a string
--------------------------------------------------------------------------------

-- Date serialization
local date = require 'date'
local date_mt = getmetatable(date())

-- Private functions

-- The original serialize function
local no_identity = { number=1, boolean=1, string=1, ['nil']=1 }

local function _serialize(x)   
   local gensym_max =  0  -- index of the gensym() symbol generator
   local seen_once  = { } -- element->true set of elements seen exactly once in the table
   local multiple   = { } -- element->varname set of elements seen more than once
   local nested     = { } -- transient, set of elements currently being traversed
   local nest_points = { }
   local nest_patches = { }
   
   local function gensym()
      gensym_max = gensym_max + 1 ;  return gensym_max
   end
   
   -- --------------------------------------------------------------------------
   -- nest_points are places where a table appears within itself, directly or not.
   -- for instance, all of these chunks create nest points in table x:
   -- "x = { }; x[x] = 1", "x = { }; x[1] = x", "x = { }; x[1] = { y = { x } }".
   -- To handle those, two tables are created by mark_nest_point:
   -- * nest_points [parent] associates all keys and values in table parent which
   --   create a nest_point with boolean `true'
   -- * nest_patches contain a list of { parent, key, value } tuples creating
   --   a nest point. They're all dumped after all the other table operations
   --   have been performed.
   --
   -- mark_nest_point (p, k, v) fills tables nest_points and nest_patches with
   -- informations required to remember that key/value (k,v) create a nest point
   -- in table parent. It also marks `parent' as occuring multiple times, since
   -- several references to it will be required in order to patch the nest
   -- points.
   -- --------------------------------------------------------------------------
   local function mark_nest_point (parent, k, v)
      local nk, nv = nested[k], nested[v]
      assert (not nk or seen_once[k] or multiple[k])
      assert (not nv or seen_once[v] or multiple[v])
      local mode = (nk and nv and "kv") or (nk and "k") or ("v")
      local parent_np = nest_points [parent]
      local pair = { k, v }
      if not parent_np then parent_np = { }; nest_points [parent] = parent_np end
      parent_np [k], parent_np [v] = nk, nv
      table.insert (nest_patches, { parent, k, v })
      seen_once [parent], multiple [parent]  = nil, true
   end
   
   -- --------------------------------------------------------------------------
   -- First pass, list the tables and functions which appear more than once in x
   -- --------------------------------------------------------------------------
   local function mark_multiple_occurences (x)
      if no_identity [type(x)] then return end
      if     seen_once [x]     then seen_once [x], multiple [x] = nil, true
      elseif multiple  [x]     then -- pass
      else   seen_once [x] = true end
      
      if type (x) == 'table' then
         nested [x] = true
         for k, v in pairs (x) do
            if nested[k] or nested[v] then mark_nest_point (x, k, v) else
               mark_multiple_occurences (k)
               mark_multiple_occurences (v)
            end
         end
         nested [x] = nil
      end
   end

   local dumped    = { } -- multiply occuring values already dumped in localdefs
   local localdefs = { } -- already dumped local definitions as source code lines


   -- mutually recursive functions:
   local dump_val, dump_or_ref_val

   -- -----------------------------------------------------------------
   -- if x occurs multiple times, dump the local var rather than the
   -- value. If it's the first time it's dumped, also dump the content
   -- in localdefs.
   -- -----------------------------------------------------------------            
   function dump_or_ref_val (x)
      if nested[x] then return 'false' end -- placeholder for recursive reference
      if not multiple[x] then return dump_val (x) end
      local var = dumped [x]
      if var then return "_[" .. var .. "]" end -- already referenced
      local val = dump_val(x) -- first occurence, create and register reference
      var = gensym()
      table.insert(localdefs, "_["..var.."]="..val)
      dumped [x] = var
      return "_[" .. var .. "]"
   end

   -- --------------------------------------------------------------------------
   -- Second pass, dump the object; subparts occuring multiple times are dumped
   -- in local variables which can be referenced multiple times;
   -- care is taken to dump locla vars in asensible order.
   -- --------------------------------------------------------------------------
   function dump_val(x)
      local  t = type(x)
      if     x==nil        then return 'nil'
      elseif t=="number"   then return tostring(x)
      elseif t=="string"   then return string.format("%q", x)
      elseif t=="boolean"  then return x and "true" or "false"
      elseif t=="function" then
         return string.format ("loadstring(%q,'@serialized')", string.dump (x))
      elseif t=="table" then
         -- Special date handling
         if getmetatable(x) == date_mt then
            return x:fmt("date(%Y, %m, %d, %H, %M, %S)")
         end

         local acc        = { }
         local idx_dumped = { }
         local np         = nest_points [x]
         for i, v in ipairs(x) do
            if np and np[v] then
               table.insert (acc, 'false') -- placeholder
            else
               table.insert (acc, dump_or_ref_val(v))
            end
            idx_dumped[i] = true
         end
         for k, v in pairs(x) do
            if np and (np[k] or np[v]) then
               --check_multiple(k); check_multiple(v) -- force dumps in localdefs
            elseif not idx_dumped[k] then
               table.insert (acc, "[" .. dump_or_ref_val(k) .. "] = " .. dump_or_ref_val(v))
            end
         end
         return "{ "..table.concat(acc,", ").." }"
      else
         error ("Can't serialize data of type "..t)
      end
   end
          
   local function dump_nest_patches()
      for _, entry in ipairs(nest_patches) do
         local p, k, v = unpack (entry)
         assert (multiple[p])
         local set = dump_or_ref_val (p) .. "[" .. dump_or_ref_val (k) .. "] = " .. 
            dump_or_ref_val (v) .. " -- rec "
         table.insert (localdefs, set)
      end
   end
   
   mark_multiple_occurences (x)
   local toplevel = dump_or_ref_val (x)
   dump_nest_patches()

   if next (localdefs) then
      return "local _={ }\n" ..
         table.concat (localdefs, "\n") .. 
         "\nreturn " .. toplevel
   else
      return "return " .. toplevel
   end
end

-- Pretty print a non-recursive table of number, string, or boolean values
-- Returns a string
local function _pprint(t, indent, prev_tables, max_width, fail_on_error)
    if t == nil then return 'nil' end
    prev_tables[t] = true

    -- Helper functions
    local function _serialize_key(k)
        local tp = type(k)
        if tp == "string" then
            if k:match("[_%a][_%a%d]*") == k then
                return k
            else
                return string.format("[%q]", k)
            end
        elseif tp == "number" or tp == "boolean" then
            return '['..tostring(k)..']'
        elseif fail_on_error then
            error("Can't serialize key type: "..tp)
        else
            return '<' .. tostring(k) .. '>'
        end
    end

    local function _serialize_value(v)
        local tp = type(v)
        if tp == "table" then
            if prev_tables[v] then
                if fail_on_error then
                    error("Recursive table found: "..tostring(v))
                else
                    return '<recursive ' .. tostring(v) .. '>'
                end
            end
            local result = _pprint(v, indent + 2, prev_tables, max_width - indent - 2, fail_on_error)
            -- Remove the indent
            return result:match('^ *([^ ].*)$')
        elseif tp == "string" then
            return string.format("%q", v)
        elseif tp == "number" or tp == "boolean" then
            return tostring(v)
        elseif fail_on_error then
            error("Can't serialize value type: "..tp)
        else
            return '<' .. tostring(v) .. '>'
        end
    end

    if type(t) ~= "table" then
        return _serialize_value(t)
    end

    if getmetatable(t) == date_mt then
        return t:fmt("date(%Y, %m, %d, %H, %M, %S)")
    end

    local ret = {}
    -- Write the array part
    local array_part = {}
    for i, v in ipairs(t) do
        array_part[i] = true
        table.insert(ret, _serialize_value(v)..',')
    end
    -- Write the hash part
    for k,v in pairs(t) do
        if not array_part[k] then
            table.insert(ret,
                _serialize_key(k)..' = '..
                    _serialize_value(v)..','
            )
        end
    end

    -- Get the length of the concatenated table
    local len = 0
    for _, s in ipairs(ret) do
        len = len + #s
    end

    -- If the representation is too long, break it up with newlines
    if len > max_width then
        -- Indent each line
        for i, s in ipairs(ret) do
            ret[i] = string.rep(" ", indent+2)..s
        end
        -- Add the braces
        table.insert(ret, 1, string.rep(" ", indent).."{")
        table.insert(ret, string.rep(" ", indent).."}")

        return table.concat(ret, "\n")
    else
        -- Add the braces
        table.insert(ret, 1, string.rep(" ", indent).."{")
        table.insert(ret, "}")
        return table.concat(ret, " ")
    end
end

-- ----------------------------------------------------------------------------
-- Public stuff
-- -----------------------------------------------------------------------------

local M = { serialize = _serialize }
-- _serialize is also a metamethod
setmetatable(M, {
    __call = function(self, obj)
        return _serialize(obj)
    end
})

--- Serialize an object.
-- @param obj The object
-- @return a string suitable for passing to `loadstring`.
-- @function serialize

--- Pretty print an object.
-- @param obj The object
-- @param[opt=80] max_width Maximum width of a line
-- @return A string representation of this object
function M.pprint(obj, max_width, fail_on_error)
    return _pprint(obj, 0, {}, max_width or 80, fail_on_error)
end

--- Write serialized data to a file.
-- @param obj The object to serialize
-- @param filename The file
-- @param[opt=true] pretty Pretty print
-- @return true or nil, error
function M.dump(obj, filename, pretty)
    local f, err = io.open(filename, 'wb')
    if not f then return nil, err end
    if pretty == nil then pretty = true end
    if pretty then
        f:write("return ")
        f:write(M.pprint(obj, 80, true)) -- Fail on error
    else
        f:write(_serialize(obj))
    end
    f:close()
    return true
end

--- Safely call a function in a protected environment.
-- @param func An untrusted function
-- @param[opt=empty] env The environment
local function safe_call(func, env)
    env = env or {}
    env.date = date
    setfenv(func, env)
    local success, result = pcall(func)
    if success then
        return result
    else
        return nil, result
    end
end

--- Safely load a string.
-- @param str An untrusted string
-- @param[opt=empty] env The environment to use
-- @return same as `loadstring`
function M.loadstring(str, env)
    local func, err = loadstring(str)
    if not func then return nil, err end
    return safe_call(func, env)
end

--- Safely load a file.
-- @param filename An untrusted file
-- @param[opt=empty] env The environment to use
-- @return same as `loadfile`
function M.loadfile(filename, env)
    local func, err = loadfile(filename)
    if not func then return nil, err end
    return safe_call(func, env)
end

return M
