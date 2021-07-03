#!/bin/lua5.3
io.write("{")
local first = true
while true do
  local chr = io.read(1)
  if chr == nil then
    io.write("}")
    return
  end
  if first == true then
    first = false
  else
    io.write(", ")
  end
  io.write(tostring(string.byte(chr)))
end
