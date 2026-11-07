local MyaC = {}

local Tokenizer = {}

function MyaC.new()
	return {
		File = {
			Content = "",
			Name = "",
		},
		Tokens = {},
		Ast = {},
		Output = "",
	}
end

function MyaC:Compile(State, FileName, OutputFile)
	local FilePtr, Error = io.open(FileName, "r")
	if not FilePtr then
		return nil, Error
	end

	State.File.Content = FilePtr:read("*a")
	State.File.Name = FileName
	FilePtr:close()
end

return MyaC
