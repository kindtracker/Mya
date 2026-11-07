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

local Ansi = {
	Reset = "\27[0m",
	Yellow = "\27[33m",
	Red = "\27[31m",
	RuaRed = "\27[38;2;255;100;100m",
	LuaBlue = "\27[38;2;100;200;255m",
}

local Logger = {}

function Logger:Error(State, ErrorType, ErrorMessageFormat, ...)
	local Format = "%sMyaC:%s %s:%d:%d: %sError:%s %s: %s\n"

	State.ErrorMessage = string.format(ErrorMessageFormat, ...)

	io.write(
		string.format(
			Format,
			Ansi.RuaRed,
			Ansi.Reset,
			State.File.Name,
			State.File.Row,
			State.File.Column,
			Ansi.Red,
			Ansi.Reset,
			ErrorType,
			State.ErrorMessage
		)
	)

	State.Stop = true

	if Rua._DevMode then
		io.write(
			string.format(
				"%sLua:%s traceback:\n%s\n",
				Ansi.LuaBlue,
				Ansi.Reset,
				debug.traceback():gsub("stack traceback:\n", ""):gsub("	", "   ")
			)
		)
	end

	if ErrorType == "SyntaxError" then
		Parser:Advance(State)
	else
		if os.exit then
			os.exit(1)
		else
			assert(false, "")
		end
	end

	return State.ErrorMessage
end

local Tokenizer = {}

function Tokenizer:Reset(State)
	Tokenizer.Character = ""
	Tokenizer.Index = 1
	Tokenizer.Token = {}
	State.File.Row = 1
	State.File.Column = 1
end

function Tokenizer:Advance(State, Step)
	Step = Step or 1

	if Step == 0 then
		Tokenizer.Character = string.sub(State.File.Content, Tokenizer.Index, Tokenizer.Index)
		return
	end

	local Direction = Step > 0 and 1 or -1

	for _ = 1, math.abs(Step) do
		Tokenizer.Index = Tokenizer.Index + Direction

		Tokenizer.Character = string.sub(State.File.Content, Tokenizer.Index, Tokenizer.Index)

		if Tokenizer.Character == "\n" then
			State.File.Row = State.File.Row + Direction
			State.File.Column = 1
		else
			State.File.Column = State.File.Column + Direction
		end
	end
end

function Tokenizer:NewToken(State)
	table.insert(State.Tokens, Tokenizer.Token)
	Tokenizer.Token = {}
end

function Tokenizer:Tokenize(State)
	Tokenizer:Advance(State, 0)

	while Tokenizer.Character ~= "" do
		print(Tokenizer.Character)

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
		elseif StringHasLetter(WhiteSpace, Tokenizer.Character) then
			Tokenizer:Advance(State)
		else
			Logger:Error(State, "SyntaxError", "Unexpected character: %q", Tokenizer.Character)
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
