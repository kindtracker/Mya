local MyaC = require("MyaC.MyaC")
local CBackend = require("MyaC.Backends.CBackend")

math.randomseed(os.time())

local State = MyaC.new()

local FileName = nil
local OutputFile = nil

local ArgumentIndex = 1

while ArgumentIndex <= #arg do
  local Argument = arg[ArgumentIndex]

  if Argument == "_DevMode" then
    MyaC._DevMode = true
  elseif Argument == "-o" then
    OutputFile = arg[ArgumentIndex + 1]
    ArgumentIndex = ArgumentIndex + 1
  else
    FileName = Argument
  end

  ArgumentIndex = ArgumentIndex + 1
end

if FileName == nil then
  return
end

MyaC:UseBackend(State, CBackend)
local Result, Error = MyaC:Compile(State, FileName, OutputFile)
