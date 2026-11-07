local lfs = require("lfs")

local SourceFolders = {
	"Source",
}

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

table.insert(SourceFolders, "Runtime/C")

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

		local Command = string.format('emcc -c -o "%s" "%s"', Object, CFile)
		os.execute(Command)
	end
end

local Objects = table.concat(ObjectFiles, " ")
local Command = string.format("emcc -o Web/C.wasm %s", Objects)
os.execute(Command)

print("Web/C.wasm")
