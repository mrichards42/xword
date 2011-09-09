-------------------------------------------------------------------------------
-- Useful functions for getting directory contents and matching them against wildcards

local lfs = require 'lfs'
local utils = require 'pl.utils'
local path = require 'pl.path'
local is_windows = path.is_windows
local tablex = require 'pl.tablex'
local attrib = lfs.attributes
local ldir = lfs.dir
local chdir = lfs.chdir
local mkdir = lfs.mkdir
local escape = utils.escape
local os,pcall,ipairs,require,_G = os,pcall,ipairs,require,_G
local remove = os.remove
local append = table.insert
local print = print
local wrap = coroutine.wrap
local yield = coroutine.yield

module 'pl.dir'

local function filemask(mask)
    mask = escape(mask)
    return mask:gsub('%%%*','.+'):gsub('%%%?','.')..'$'
end

--- does the filename match the shell pattern?.
-- (cf. fnmatch.fnmatch in Python, 11.8)
-- @param file A file name
-- @param pattern A shell pattern
function fnmatch(file,pattern)
    return path.normcase(file):find(filemask(pattern)) ~= nil
end

--- return a list of all files in a list of files which match the pattern.
-- (cf. fnmatch.filter in Python, 11.8)
-- @param files A table containing file names
-- @param pattern A shell pattern.
function filter(files,pattern)
    local res = {}
    local mask = filemask(pattern)
    for i,f in ipairs(files) do
        if f:find(mask) then append(res,f) end
    end
    return res
end

function _listfiles(dir,filemode,match)
    local res = {}
    if not dir then dir = '.' end
    for f in ldir(dir) do
        if f ~= '.' and f ~= '..' then
            local p = path.join(dir,f)
            local mode = attrib(p,'mode')
            if mode == filemode and (not match or match(p)) then
                append(res,p)
            end
        end
    end
    return res
end

--- return a list of all files in a directory which match the a shell pattern.
-- @param dir A directory. If not given, all files in current directory are returned.
-- @param mask  A shell pattern. If  not given, all files are returned.
function getfiles(dir,mask)
    local match
    if mask then
        mask = filemask(mask)
        match = function(f)
            return f:find(mask)
        end
    end
    return _listfiles(dir,'file',match)
end

--- return a list of all subdirectories of the directory.
-- @param dir A directory
function getdirectories(dir)
    return _listfiles(dir,'directory')
end

function _dirfiles(dir)
    local dirs = {}
    local files = {}
    for f in ldir(dir) do
        if f ~= '.' and f ~= '..' then
            local p = path.join(dir,f)
            local mode = attrib(p,'mode')
            if mode=='directory' then
                append(dirs,f)
            else
                append(files,f)
            end
        end
    end
    return dirs,files
end

local alien,no_alien,kernel,CopyFile,MoveFile

local function file_op (is_copy,src,dest,flag)
    if is_windows then
        local res
        -- if we haven't tried to load Alien before, then do so
        if not alien and not no_alien then
            res,alien,err = pcall(require,'alien')
            no_alien = not res
            if alien then
                -- register the Win32 CopyFile and MoveFile functions
                local spec = {'string','string','int',ret='int',abi='stdcall'}
                kernel = alien.load('kernel32.dll')
                CopyFile = kernel.CopyFileA
                CopyFile:types(spec)
                MoveFile = kernel.MoveFileA
                MoveFile:types(spec)
            end
        end
        -- fallback if there's no Alien, just use DOS commands *shudder*
        if not CopyFile then
            cmd = is_copy and 'copy' or 'remove'
        else
            if is_copy then return CopyFile(src,dest,flag)
            else return MoveFile(src,dest) end
        end
    else -- for Unix, just use cp for now
        cmd = is_copy and 'cp' or 'mv'
    end
    cmd = cmd..' '..src..' '..dest
    return os.execute(cmd) ~= 0
end

--- copy a file.
-- @param src source file
-- @param dest destination file
-- @param flag true if you want to force the copy (default)
-- @return true if operation succeeded
function copyfile (src,dest,flag)
    flag = flag==nil or flag
    return file_op(true,src,dest,flag and 0 or 1)==1  --fix flag sense wrong
end

--- move a file.
-- @param src source file
-- @param dest destination file
-- @return true if operation succeeded
function movefile (src,dest)
    return file_op(false,src,dest,0)==1
end


function _walker(root,bottom_up)
    local dirs,files = _dirfiles(root)
    if not bottom_up then yield(root,dirs,files) end
    for i,d in ipairs(dirs) do
        _walker(root..path.sep..d,bottom_up)
    end
    if bottom_up then yield(root,dirs,files) end
end

-- return an iterator which walks through a directory tree starting at root.
-- The iterator returns (root,dirs,files)
-- Note that dirs and files are lists of names (i.e. you must say _path.join(root,d)_
-- to get the actual full path)
-- If bottom_up is false (or not present), then the entries at the current level are returned
-- before we go deeper. This means that you can modify the returned list of directories before
-- continuing.
-- This is a clone of os.walk from the Python libraries.
-- @param root A starting directory
-- @param bottom_up False if we start listing entries immediately.
function walk(root,bottom_up)
    return wrap(function () _walker(root,bottom_up) end)
end

--- remove a whole directory tree.
-- @param path A directory path
function rmtree(fullpath)
    for root,dirs,files in walk(fullpath) do
        for i,f in ipairs(files) do
            remove(path.join(root,f))
        end
        lfs.rmdir(root)
    end
end

local dirpat
if path.is_windows then
    dirpat = '(.+)\\[^\\]+$'
else
    dirpat = '(.+)/[^/]+$'
end

--- create a directory path.
-- This will create subdirectories as necessary!
-- @param path A directory path
function makepath(p)
   p = path.normcase(p)
  if #p == 2 and p:sub(2,2) == ':' then return true end
  if not path.isdir(p) then
    local subp = p:match(dirpat)
    if not makepath(subp) then return false end
    return lfs.mkdir(p)
  else
    return true
  end
end


--- clone a directory tree. Will always try to create a new directory structure
-- if necessary.
-- @param path1 the base path of the source tree
-- @param path2 the new base path for the destination
-- @param file_fun an optional function to apply on all files
-- @return if failed, false plus an error message. If completed the traverse,
--  true, a list of failed directory creations and a list of failed file operations.
-- @usage clonetree('.','../backup',copyfile)
function clonetree (path1,path2,file_fun,verbose)
    local abspath,normcase,isdir,join = path.abspath,path.normcase,path.isdir,path.join
    local faildirs,failfiles = {},{}
    if not isdir(path1) then return false,'source is not a valid directory' end
    path1 = abspath(normcase(path1))
    path2 = abspath(normcase(path2))
    if verbose then verbose('normalized:',path1,path2) end
    -- particularly NB that the new path isn't fully contained in the old path
    if path1 == path2 then return false,"paths are the same" end
    local i1,i2 = path2:find(path1,1,true)
    if i2 == #path1 and path2:sub(i2+1,i2+1) == path.sep then
        return false,'destination is a subdirectory of the source'
    end
    local cp = path.common_prefix (path1,path2)
    local idx = #cp
    if idx == 0 then -- no common path, but watch out for Windows paths!
        if path1:sub(2,2) == ':' then idx = 3 end
    end
    for root,dirs,files in walk(path1) do
        local opath = path2..root:sub(idx)
        if verbose then verbose('paths:',opath,root) end
        if not isdir(opath) then
            local ret = makepath(opath)
            if not ret then append(faildirs,opath) end
            if verbose then verbose('creating:',opath,ret) end
        end
        if file_fun then
            for i,f in ipairs(files) do
                local p1 = join(root,f)
                local p2 = join(opath,f)
                local ret = file_fun(p1,p2)
                if not ret then append(failfiles,p2) end
                if verbose then
                    verbose('files:',p1,p2,ret)
                end
            end
        end
    end
    return true,faildirs,failfiles
end
