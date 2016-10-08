local path = require 'pl.path'
local makepath = require 'pl.dir'.makepath
local serialize = require "serialize"

-- Default config
graph.config = {
    color = {
        correct = wx.wxColour(wx.wxGREEN),
        incorrect = wx.wxColour(wx.wxRED),
        black = wx.wxColour(wx.wxBLACK),
        blank = wx.wxColour(wx.wxLIGHT_GREY)
    }
}

graph.configdir = path.join(xword.configdir, 'graph')

function graph.get_config_filename()
    return path.join(graph.configdir, 'config.lua')
end

function graph.load_config()
    local config = serialize.loadfile(graph.get_config_filename())
    if type(config) ~= 'table' then
        config = {}
    end
    -- Load colors
    for k, _ in pairs(graph.config.color) do
        local color = config.color[k]
        if color then
            graph.config.color[k]:Set(color)
        end
    end
end

function graph.save_config()
    local config = { color = {} }
    -- Save colors
    for k, color in pairs(graph.config.color) do
        config.color[k] = color:GetAsString()
    end
    -- Dump
    if not path.isdir(graph.configdir) then
        makedirs(graph.configdir)
    end
    serialize.dump(config, graph.get_config_filename(), true)
end
