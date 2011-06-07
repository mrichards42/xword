-- ===========================================================================
-- Database functions
-- ===========================================================================

require 'lsqlite3'
require 'date'
local makepath = require 'pl.dir'.makepath
local path = require 'pl.path'
local tablex = require 'pl.tablex'

local P = download.database

-- ---------------------------------------------------------------------------
-- Open / close
-- ---------------------------------------------------------------------------

function P.open(dbfile)
    if rawget(P, "__db") then
        return P.__db
    end

    if not dbfile then
        dbfile = P.dbfilename
    end
    makepath(path.dirname(dbfile))

    -- ----------------------------------------------------------------------------
    -- Open or create the database
    -- ----------------------------------------------------------------------------
    local db
    if not lfs.attributes(dbfile) then
        -- Create the database, it doesn't exist
        db = assert(sqlite3.open(dbfile))
        db:exec([[
            CREATE TABLE puz_index (
                id         INTEGER PRIMARY KEY,
                filename   VARCHAR,
                date       DATE,
                source     VARCHAR,
                modified   INTEGER,
                started    BOOLEAN,
                complete   REAL,
                time       INTEGER
            );
        ]])
    else
        db = assert(sqlite3.open(dbfile))
    end

    P.__db = db
    return db
end

function P.close()
    P.stmts.__cached = nil
    P.__db:close_vm()
    P.__db:close()
    P.__db = nil
end

-- ---------------------------------------------------------------------------
-- Conversions from lua to sqlite
-- ---------------------------------------------------------------------------

-- Date conversion
local function db_to_date(str) return date(str) end
local function date_to_db(date) return date:fmt('%Y-%m-%d') end

-- Bool conversion
local function db_to_bool(num) return num == 1 and true or false end
local function bool_to_db(val) return val and 1 or 0 end


-- Do all conversion to and from the database
local function table_to_db(row)
    row = tablex.copy(row)
    if row.date then
        row.date = date_to_db(row.date)
    end
    if row.started ~= nil then
        row.started = bool_to_db(row.started)
    end
    return row
end

local function db_to_table(row)
    if row.date then
        row.date = db_to_date(row.date)
    end
    if row.started then
        row.started = db_to_bool(row.started)
    end
    return row
end

-- ---------------------------------------------------------------------------
-- SQL statement cache
-- ---------------------------------------------------------------------------
P.stmts = {}
local stmts = P.stmts
stmts.__cached = {}
stmts.__strings = {}

local mt = {}
-- Compile each SQL statement on first use, then cache it
function mt:__index(name)
    if not self.__cached[name] then
        self.__cached[name] = P.__db:prepare(self.__strings[name])
    end
    return self.__cached[name]
end
setmetatable(stmts, mt)


-- ----------------------------------------------------------------------------
-- Select a record from the database
-- ----------------------------------------------------------------------------

stmts.__strings.select_filename =
    "SELECT * FROM puz_index WHERE filename LIKE ?"
stmts.__strings.select_name_date =
    "SELECT * FROM puz_index WHERE source = ? AND date = ?"
stmts.__strings.select_id =
    "SELECT * FROM puz_index WHERE id = ?"

-- Select the next record and return it as a table
local function select_next(stmt)
    -- Return nothing if there are no matches
    if stmt:step() ~= sqlite3.ROW then
        stmt:reset()
        return
    end
    local row = stmt:get_named_values()

    -- Convert sqlite values to lua values, and add a commit() function to the
    -- table
    return db_to_table(row)
end

-- Select all matching records and return them as a table of tables
local function select_all(stmt, ...)
    stmt:bind_values(unpack(arg))
    local ret = {}
    while true do
        local row = select_next(stmt)
        if not row then break end
        table.insert(ret, row)
    end
    stmt:reset()
    return ret
end

-- Select a record and convert types
local function select_one(stmt, ...)
    stmt:bind_values(unpack(arg))
    local ret = select_next(stmt)
    stmt:reset()
    return ret
end

-- find(filename)
-- find(sourcename, date)
function P.find(...)
    if #arg == 1 then
        return select_all(stmts.select_filename, arg[1])
    elseif #arg == 2 then
        return select_all(stmts.select_name_date, arg[1], date_to_db(arg[2]))
    end
end

-- Error if more than one record matches
function P.find_one(...)
    local ret = P.find(unpack(arg))
    assert(#ret <= 1, string.format("Not a unique match (%s) [%d matche(s)]", arg[1], #ret))
    return ret[1]
end

function P.last()
    -- Return a database-enabled table of the last insert
    local id = P.__db:last_insert_rowid()
    if id ~= 0 then
        return select_one(stmts.select_id, id)
    end
end



-- ----------------------------------------------------------------------------
-- Insert a record into the database
-- ----------------------------------------------------------------------------

stmts.__strings.insert = [[
    INSERT INTO puz_index VALUES (
        NULL,
        :filename,
        :date,
        :source,
        :modified,
        :started,
        :complete,
        :time );
]]

-- insert{filename = "filename", date = date(), source = "source" ...}
-- insert("filename", date(), "source", ...)
function P.insert(...)
    if #arg == 1 then
        local options = arg[1]
        assert(type(options) == 'table')
        stmts.insert:bind_names(table_to_db(options))
    else
        assert(#arg == 7)
        arg[2] = date_to_db(arg[2]) -- date
        arg[5] = bool_to_db(arg[5]) -- started
        stmts.insert:bind_values(unpack(arg))
    end
    stmts.insert:step()
    stmts.insert:reset()
end


-- ----------------------------------------------------------------------------
-- Update a record in the database
-- ----------------------------------------------------------------------------

-- update{ filename = "filename", source = "source", ... }
-- Must supply either filename or source and date
function P.update(options)
    local options = table_to_db(options)

    -- WHERE clause
    local where
    if options.id then
        where = "WHERE id = :id"
    elseif options.filename then
        where = "WHERE filename = :filename"
    elseif options.source and options.date then
        where = "WHERE source = :source AND date = :date"
    else -- Can't update without filename or (source and date)
        return
    end

    -- SET columns
    local set = {}
    for _, col in ipairs({
        "filename", "date", "source", "modified", "started", "complete", "time"
    }) do
        if options[col] then
            table.insert(set, string.format('%s = :%s', col, col))
        end
    end

    local stmt = assert(P.__db:prepare(
        "UPDATE puz_index SET".." "..table.concat(set, ', ').." "..where
    ))
    stmt:bind_names(options)
    stmt:step()
    stmt:reset()
end


-- ----------------------------------------------------------------------------
-- Delete a record from the database
-- ----------------------------------------------------------------------------

stmts.__strings.delete_id =
    "DELETE FROM puz_index WHERE id = ?"
stmts.__strings.delete_filename =
    "DELETE FROM puz_index WHERE filename = ?"
stmts.__strings.delete_name_date =
    "DELETE FROM puz_index WHERE source = ? AND date = ?"

local function delete_record(stmt, ...)
    stmt:bind_values(unpack(arg))
    stmt:step()
    stmt:reset()
end

-- delete(id)
-- delete(filename)
-- delete(sourcename, date)
function P.delete(...)
    if #arg == 1 then
        if type(arg[1]) == 'string' then
            return delete_record(stmts.delete_filename, arg[1])
        else
            return delete_record(stmts.delete_id, arg[1])
        end
    elseif #arg == 2 then
        return delete_record(stmts.delete_name_date, arg[1], date_to_db(arg[2]))
    end
end

-- ----------------------------------------------------------------------------
-- List the database contents (debug)
-- ----------------------------------------------------------------------------
local function align(str, n)
    local n = n or 20
    local str = tostring(str)
    return str .. string.rep(' ', n - #str)
end

function P.list()
    print(align('id', 5)..align('filename', 12)..align('date', 12)..align('source', 15)..align('modified', 12)..align('started', 10)..align('complete', 10)..align('time', 10))
    print(string.rep('-',4)..' '..string.rep('-',11)..' '..string.rep('-',11)..' '..string.rep('-',14)..' '..string.rep('-',11)..' '..string.rep('-',9)..' '..string.rep('-',9)..' '..string.rep('-',9)..' ')
    for row in P.__db:nrows("SELECT * FROM puz_index") do
        print(align(row.id, 5)..align(row.filename, 12)..align(row.date, 12)..align(row.source, 15)..align(row.modified, 12)..align(row.started, 10)..align(row.complete, 10)..align(row.time, 10))
    end
end
