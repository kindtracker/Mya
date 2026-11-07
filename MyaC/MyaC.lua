local MyaC = {}

local WhiteSpace = "\n\t\r "

local function TableHasString(Table, Value)
	for _, Item in pairs(Table) do
		if type(Item) == "string" and Item == Value then
			return true
		end
	end

	return false
end

local function StringHasLetter(String, Letter)
	return Letter ~= "" and string.find(String, Letter, 1, true) ~= nil
end

local function PrintTable(Value, Indent)
	Indent = Indent or 0

	for Key, Item in pairs(Value) do
		io.write(string.rep("  ", Indent))
		if type(Key) == "string" then
			io.write(string.format("[%q] = ", Key))
		else
			io.write(string.format("[%s] = ", Key))
		end

		if type(Item) == "table" then
			io.write("{\n")
			PrintTable(Item, Indent + 1)
			io.write(string.rep("  ", Indent) .. "}\n")
		else
			if type(Item) == "string" then
				io.write(string.format("%q\n", Item))
			else
				io.write(string.format("%s\n", Item))
			end
		end
	end
end

local Tokenizer = {}

function Tokenizer:Reset(State)
	Tokenizer.Character = ""
	Tokenizer.Index = 1
	Tokenizer.Token = {}
end

function Tokenizer:Advance(State, Step)
	Step = Step or 1

	Tokenizer.Index = Tokenizer.Index + Step
	Tokenizer.Character = string.sub(State.File.Content, Tokenizer.Index, Tokenizer.Index)
end

function Tokenizer:NewToken(State)
	table.insert(State.Tokens, Tokenizer.Token)
	Tokenizer.Token = {}
end

function Tokenizer:Tokenize(State)
	Tokenizer:Advance(State, 0)

	while Tokenizer.Character ~= "" do
		if Tokenizer.Character == "<" then
			Tokenizer.Token = {
				Kind = "TagOpen",
			}

			Tokenizer:NewToken(State)
			Tokenizer:Advance(State)
		elseif Tokenizer.Character == ">" then
			Tokenizer.Token = {
				Kind = "TagClose",
			}

			Tokenizer:NewToken(State)
			Tokenizer:Advance(State)
		else
			StringHasLetter(WhiteSpace, Tokenizer.Character)
			Tokenizer:Advance(State)
		end
	end
end

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

function MyaC:Tokenize(State)
	Tokenizer:Reset(State)
	Tokenizer:Tokenize(State)
end

function MyaC:Compile(State, FileName, OutputFile)
	local FilePtr, Error = io.open(FileName, "r")
	if not FilePtr then
		return nil, Error
	end

	State.File.Content = FilePtr:read("*a")
	State.File.Name = FileName
	FilePtr:close()

	MyaC:Tokenize(State)

	PrintTable(State.Tokens)
end

return MyaC
