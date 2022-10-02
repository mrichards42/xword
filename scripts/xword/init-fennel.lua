-- Setup fennel
local fennel = require 'fennel'
table.insert(package.loaders or package.searchers, fennel.searcher)
fennel.path = fennel.path .. ';' .. package.path:gsub(".lua", ".fnl")
