local Mya = require("Mya.Mya")
local CBackend = require("Mya.CGenerator")

math.randomseed(os.time())

local State = Mya.new()

local FileName = nil
local OutputFile = nil

local ArgumentIndex = 1

while ArgumentIndex <= #arg do
  local Argument = arg[ArgumentIndex]

  if Argument == "_DevMode" then
    Mya._DevMode = true
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

Mya:UseBackend(State, CBackend)
local Result, Error = Mya:Compile(State, FileName, OutputFile)
