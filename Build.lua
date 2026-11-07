local lfs = require("lfs")

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
				table.insert(Files, FullPath)
			end
		end
	end

	return Files
end

lfs.mkdir("Build/")

local CFiles = ListAllFiles("Source")
local ObjectFiles = {}

for _, CFile in ipairs(CFiles) do
	lfs.mkdir("Build/" .. "Source")

	local Object = "Build/" .. CFile:gsub("%.c", ".o")
	table.insert(ObjectFiles, Object)

	local Command = string.format("emcc -c -o %s %s", Object, CFile)
	os.execute(Command)
end

local Objects = table.concat(ObjectFiles, " ")
local Command = string.format("emcc -o Web/C.wasm %s", Objects)
os.execute(Command)
