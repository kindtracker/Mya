local lfs = require("lfs")

local SourceFolders = {
	"Web/C",
}

local IncludeFolders = {}

-- Custom / Release / Debug
-- Release will add "-O2"
-- Debug will add CFlag "-O0", LDFlag "-sASSERTIONS=2", and Sanitizer "address,undefined"
-- Custom does nothing
local BuildMode = "Debug"

local Sanitizers = "" -- e.g. "address,undefined"
local CFlags = "-Wall -Wextra"
local LDFlags = ""

function ListAllFiles(Directory)
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
				if File:match("%.c$") then
					table.insert(Files, FullPath)
				end
			end
		end
	end

	return Files
end

function Execute(Command)
	-- print("> " .. Command)

	local Success, Type, Code = os.execute(Command)
	if not Success then
		error(string.format("Command failed (exit code: %s): %s", tostring(Code), Command))
	end

	return Success
end

if BuildMode == "Release" then
	CFlags = CFlags .. " -O2"
elseif BuildMode == "Debug" then
	CFlags = CFlags .. " -O0 -g"
	LDFlags = LDFlags .. " -sASSERTIONS=2"
	Sanitizers = Sanitizers .. "address,undefined"
end

CFlags = CFlags .. " -fsanitize=" .. Sanitizers
LDFlags = LDFlags .. " -fsanitize=" .. Sanitizers

lfs.mkdir("Build")
local ObjectFiles = {}
for _, SourceFolder in ipairs(SourceFolders) do
	local Sources = ListAllFiles(SourceFolder)

	for _, CFile in ipairs(Sources) do
		print(CFile)

		local Object = "Build/" .. CFile:gsub("%.c$", ".o")
		table.insert(ObjectFiles, Object)

		local Parent = Object:match("(.+)/[^/]+$")
		if Parent then
			os.execute('mkdir -p "' .. Parent .. '"')
		end

		local Command = string.format("cc -c -o %q %q %s", Object, CFile, CFlags)
		Execute(Command)
	end
end

--[[
local Objects = table.concat(ObjectFiles, " ")
local Command = string.format("emcc -o Web/C.js %s %s", Objects, LDFlags)
Execute(Command)

print("Web/C.wasm")
]]
