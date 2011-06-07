local serialize = require 'serialize'
local lfs = require 'lfs'
local join = require 'pl.path'.join
local startswith = require 'pl.stringx'.startswith

local function gen_packages(outdir)
    -- Walk the xword.scriptsdir directory
    local packages = {
        xword = {
            name = "XWord",
            version = xword.version,
            
        }
    }
    for name in lfs.dir(xword.scriptsdir) do
        local dirname = join(xword.scriptsdir, name)
        if not startswith(name, '.') and name ~= 'xword' and name ~= 'libs'
            and lfs.attributes(dirname, 'mode') == 'directory'
        then
            local info = serialize.loadfile(join(dirname, 'info.lua'))
            if info then
                if info.name and info.requires and info.version then
                    -- Add info to the package table
                    info.packagename = name
                    info.download = string.format("http://sourceforge.net/projects/wx-xword/files/lua/%s_%s.tar.gz", info.packagename, info.version)
                    table.insert(packages, info)
                    -- Pack the file in a tar
                    local archive_name = join(outdir, name..'_'..info.version)
                    local rc = os.execute(string.format(
                        "%s a -r -x!.* %s.tar %s/*",
                        join(xword.scriptsdir, 'xworddebug', '7za.exe'),
                        archive_name,
                        join(xword.scriptsdir, name)
                    ))
                    if rc ~= 0 then
                        print("Error compressing files for package "..name)
                    else
                        -- Zip the tar
                        local rc = os.execute(string.format(
                            "%s a %s.tar.gz %s.tar",
                            join(xword.scriptsdir, 'xworddebug', '7za.exe'),
                            archive_name,
                            archive_name
                        ))
                        if rc ~= 0 then
                            print("Error compressing files for package "..name)
                            os.remove(archive_name..'.tar.gz')
                        else
                            print("Package "..name.." compressed to "..archive_name..".tar.gz")
                        end
                    end
                    -- Remove the tar
                    os.remove(archive_name..'.tar')
                else
                    print("Package "..name.." has an incomplete info.lua file")
                end
            else
                print("Package "..name.." is missing info.lua")
            end
        end
    end
    serialize.pdump(packages, join(outdir, 'packages.lua'))
    print("Packages written to "..join(outdir, 'packages.lua'))
end

local function init()
    xword.frame:AddMenuItem(
        {'Debug'}, "Generate Package table and archives",
        function(evt)
            local dirname = wx.wxDirSelector(
                "Select a directory for package output",
                xword.userdatadir
            )
            if dirname and dirname ~= '' then
                gen_packages(dirname)
            end
        end
    )
end

if xword.frame then
    init()
else -- command line
    lfs.mkdir([[D:\C++\xword\trunk\packages]])
    gen_packages([[D:\C++\xword\trunk\packages]])
end
