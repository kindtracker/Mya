local lfs = require("lfs")

local SourceFolders = {}

local function ListAllFiles(Directory)
	local Files = {}

	for File in lfs.dir(Directory) do
		if File ~= "." and File ~= ".." then
			local FullPath = Directory .. "/" .. File
			local Attributes = lfs.attributes(FullPath)

			if Attributes.mode == "directory" then
				local SubFiles = ListAllFiles(FullPath)

				for _, SubFile in ipairs(SubFiles) do
					table.insert(Files, SubFile)
				end
			else
				if File:match("%.html$") then
					table.insert(Files, FullPath)
				end
			end
		end
	end

	return Files
end

local function CopyFile(Source, Destination)
	local InputPtr = assert(io.open(Source, "rb"))
	local OutputPtr = assert(io.open(Destination, "wb"))
	local FileContent = InputPtr:read("*a")
	OutputPtr:write(FileContent)
	InputPtr:close()
	OutputPtr:close()
end

function Execute(Command)
	-- print("> " .. Command)

	local Success, Type, Code = os.execute(Command)
	if not Success then
		error(string.format("Command failed (exit code: %s): %s", tostring(Code), Command))
	end

	return Success
end

table.insert(SourceFolders, "Runtime/Html")

for _, SourceFolder in ipairs(SourceFolders) do
	local Sources = ListAllFiles(SourceFolder)

	for _, File in ipairs(Sources) do
		local Destination

		if SourceFolder == "Runtime/Html" then
			local Filename = File:match("[^/]+$")
			Destination = "Web/" .. Filename
		else
			Destination = "Web/" .. File
		end

		local Parent = Destination:match("(.+)/[^/]+$")
		if Parent then
			Execute('mkdir -p "' .. Parent .. '"')
		end

		local Command = string.format("lua MyaC/Main.lua -o %s %s", Destination, File)
		Execute(Command)
	end
end
