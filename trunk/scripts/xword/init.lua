-- ============================================================================
-- XWord additions
--     This package is automatically loaded by XWord
-- ============================================================================

require 'xword.utils'

-- Require all files in a given directory that are not named init.lua
-- Optionally, also attempt to require subdirs as packages (by subdir name)
require 'lfs'
function xword.requiredir(dirname, packages)
    local dirname = assert(dirname)
    if dirname ~= '' then dirname = dirname .. '/' end
    local pkgname = dirname:gsub('[\\/]', '.')

    -- iterate all files and directories
    for script in lfs.dir(xword.scriptsdir..dirname) do
        if script:sub(1,1) ~= '.' and script ~= 'init.lua' then
            -- Is this a file or a directory?
            local attr = lfs.attributes(xword.scriptsdir..dirname..script)
            if attr.mode == 'file' and script:sub(-4) == '.lua' then
                print ('file:'..script)
                require(pkgname..script:sub(1,-5))
            elseif packages and attr.mode == 'directory' then
                print ('dir:'..script)
                -- Is there an init file in the directory?
                if lfs.attributes(xword.scriptsdir..dirname..script..'/init.lua') then
                    require(pkgname..script)
                end
            end
        end
    end
end

-- Require the scripts directory, including subdirs
xword.requiredir('', true)