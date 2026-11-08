local lfs = require("lfs")

local SourceFolders = { "Web/Html" }

local OutputFolder = "Output"
local BuildFolder = "Build"
local Compiler = "gcc"

local function ListAllFiles(Directory, Extension)
  local Files = {}

  for File in lfs.dir(Directory) do
    if File ~= "." and File ~= ".." then
      local FullPath = Directory .. "/" .. File
      local Attributes = lfs.attributes(FullPath)

      if Attributes.mode == "directory" then
        local SubFiles = ListAllFiles(FullPath, Extension)

        for _, SubFile in ipairs(SubFiles) do
          table.insert(Files, SubFile)
        end
      elseif File:match("%." .. Extension .. "$") then
        table.insert(Files, FullPath)
      end
    end
  end

  return Files
end

local function Execute(Command)
  print("> " .. Command)

  local Success, Type, Code = os.execute(Command)

  if not Success then
    error(string.format("Command failed (exit code: %s): %s", tostring(Code), Command))
  end

  return Success
end

local function MakeDirectory(Directory)
  Execute('mkdir -p "' .. Directory .. '"')
end

local function CopyFile(Source, Destination)
  local InputPtr = assert(io.open(Source, "rb"))
  local OutputPtr = assert(io.open(Destination, "wb"))

  OutputPtr:write(InputPtr:read("*a"))

  InputPtr:close()
  OutputPtr:close()
end

MakeDirectory(OutputFolder)
MakeDirectory(BuildFolder)

for _, SourceFolder in ipairs(SourceFolders) do
  local Sources = ListAllFiles(SourceFolder, "html")

  for _, File in ipairs(Sources) do
    local Destination = OutputFolder .. "/" .. File .. ".c"
    local Parent = Destination:match("(.+)/[^/]+$")

    if Parent then
      MakeDirectory(Parent)
    end

    Execute(string.format('luajit MyaC/Main.lua -o "%s" "%s"', Destination, File))
  end
end

local CFiles = ListAllFiles(OutputFolder, "c")
local ObjectFiles = {}
for _, File in ipairs(CFiles) do
  local ObjectFile = BuildFolder .. "/" .. File:sub(#OutputFolder + 2):gsub("%.c$", ".o")

  local Parent = ObjectFile:match("(.+)/[^/]+$")
  if Parent then
    MakeDirectory(Parent)
  end

  Execute(string.format('%s -c "%s" -o "%s" -IRuntime', Compiler, File, ObjectFile))

  table.insert(ObjectFiles, ObjectFile)
end

local RuntimeObject = BuildFolder .. "/Mya.o"
local RuntimeMainObject = BuildFolder .. "/MyaMain.o"

Execute(string.format('%s -c Runtime/Mya.c -o "%s" -I.', Compiler, RuntimeObject))
table.insert(ObjectFiles, RuntimeObject)

Execute(string.format('%s -c Runtime/Main.c -o "%s" -I.', Compiler, RuntimeMainObject))
table.insert(ObjectFiles, RuntimeMainObject)

Execute(string.format('%s -lSDL3 %s -o "%s"', Compiler, table.concat(ObjectFiles, " "), BuildFolder .. "/Mya"))
print(BuildFolder .. "/Mya")
