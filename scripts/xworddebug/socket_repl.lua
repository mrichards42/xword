local M = {}
local tablex = require('pl.tablex')
local pretty = require('pl.pretty')

-- Basic REPL --

function M.detectcontinue(err)
  return err:match("'<eof>'$") or err:match("<eof>$")
end

function M.repr(obj)
  if type(obj) == "table" then
    return pretty.write(obj)
  else
    return tostring(obj)
  end
end

function M.eval(str)
  local func, err = loadstring('return ' .. str)
  if not func then
    func, err = loadstring(str)
  end
  if not func then
    return nil, err
  else
    local success, res = pcall(func)
    if not success then
      return nil, res
    end
    return res
  end
end

function M.eval_with_printer(print_fn, str)
  -- redefine print just for this function call
  local oldprint = print
  print = print_fn
  local res, err = M.eval(str)
  print = oldprint
  return res, err
end

-- wxSocket stuff  --

function M.read_input(sock)
  local input = ""
  local chunk_size = 1024
  while true do
    local chunk = sock:Read(chunk_size)
    input = input .. chunk:sub(0, sock:LastCount())
    if sock:LastCount() < chunk_size then
      return input
    end
  end
end

function M.print_to_socket(sock)
  return function(...)
    for _, v in ipairs({...}) do
      sock:Write(M.repr(v))
      sock:Write("\t")
    end
    sock:Write("\n")
  end
end

function M.on_connect(event)
  local sock = M.server:Accept(false)
  print("Connected", sock)
  sock:SetEventHandler(M.event_handler)
  sock:SetNotify(wx.wxSOCKET_INPUT_FLAG + wx.wxSOCKET_LOST_FLAG);
  sock:Notify(true)
  sock:Write("Connected to XWord!\n")
  sock:Write("> ")
end

function M.on_input(event)
  local sock = event.Socket
  local print_fn = M.print_to_socket(sock)
  -- read
  local s = M.read_input(sock)
  -- eval
  local result, err = M.eval_with_printer(print_fn, s)
  if err and M.detectcontinue(err) then
    -- incomplete line: wait for more
    sock:Unread(s)
    sock:Write(">> ")
  else
    -- print
    print_fn(result or err)
    sock:Write("\n")
    sock:Write("> ")
  end
end

function M.on_disconnect(event)
  print("Disconnected", event.Socket)
end

function M.socket_handler(event)
  if event:GetSocketEvent() == wx.wxSOCKET_CONNECTION then
    M.on_connect(event)
  elseif event:GetSocketEvent() == wx.wxSOCKET_INPUT then
    M.on_input(event)
  elseif event:GetSocketEvent() == wx.wxSOCKET_LOST then
    M.on_disconnect(event)
  end
end

function M.create_server(port, event_handler)
  print("Creating server on port", port)
  local addr = wx.wxIPV4address()
  addr:Service(port)
  local server = wx.wxSocketServer(addr, wx.wxSOCKET_REUSEADDR)
  M.server = server
  M.event_handler = event_handler
  if server:Ok() then
    print("Setting up event listeners")
    event_handler:Connect(wx.wxEVT_SOCKET, M.socket_handler)
    server:SetEventHandler(event_handler)
    server:SetNotify(wx.wxSOCKET_CONNECTION_FLAG);
    server:Notify(true)
  else
    print("Server not ok!")
  end
end

function M.init()
  xword.frame:AddMenuItem({'Debug'}, 'Start Socket REPL...',
    function(evt)
      local port = wx.wxGetNumberFromUser(
        'Enter a port number to use for the socket REPL.',
        '',
        'Start Socket REPL',
        12345,  -- default
        1000,   -- min
        99999)  -- max
      if port ~= -1 then
        M.create_server(port, xword.frame)
      end
    end
    )
end

return M
