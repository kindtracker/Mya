local MyaC = {}

local BaseDigits = "0123456789"
local BaseLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
local BaseIdent = BaseLetters .. BaseDigits .. "_-:"
local WhiteSpace = "\n\t\r "

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
		elseif type(Item) == "string" then
			io.write(string.format("%q\n", Item))
		else
			io.write(string.format("%s\n", tostring(Item)))
		end
	end
end

local Ansi = {
	Reset = "\27[0m",
	Red = "\27[31m",
	RuaRed = "\27[38;2;255;100;100m",
	LuaBlue = "\27[38;2;100;200;255m",
}

local Logger = {}

function Logger:Error(State, ErrorType, ErrorMessage, Row, Column)
	Row = Row or State.File.Row
	Column = Column or State.File.Column

	State.Errors = State.Errors or {}

	local Error = {
		Type = ErrorType,
		Message = ErrorMessage,
		Row = Row,
		Column = Column,
	}

	table.insert(State.Errors, Error)

	State.ErrorMessage = ErrorMessage
	State.Stop = true

	local Format = "%sMyaC:%s %s:%d:%d: %sError:%s %s: %s\n"

	io.write(
		string.format(
			Format,
			Ansi.RuaRed,
			Ansi.Reset,
			State.File.Name,
			Row,
			Column,
			Ansi.Red,
			Ansi.Reset,
			ErrorType,
			ErrorMessage
		)
	)

	if MyaC._DevMode then
		io.write(
			string.format(
				"%sLua:%s traceback:\n%s\n",
				Ansi.LuaBlue,
				Ansi.Reset,
				debug.traceback():gsub("stack traceback:\n", ""):gsub("\t", "   ")
			)
		)
	end

	return Error
end

local Tokenizer = {}

function Tokenizer:Reset(State)
	self.Character = ""
	self.Index = 0

	self.Token = {}
	self.Tokens = {}

	State.Tokens = {}
	State.Errors = {}
	State.Stop = false

	State.File.Row = 1
	State.File.Column = 1
end

function Tokenizer:Advance(State, Step)
	Step = Step or 1

	if Step == 0 then
		self.Character = State.File.Content:sub(self.Index, self.Index)
		return
	end

	local Direction = Step > 0 and 1 or -1

	for _ = 1, math.abs(Step) do
		self.Index = self.Index + Direction

		self.Character = State.File.Content:sub(self.Index, self.Index)

		if self.Character == "\n" then
			State.File.Row = State.File.Row + Direction
			State.File.Column = 1
		else
			State.File.Column = State.File.Column + Direction
		end
	end
end

function Tokenizer:NewToken(State, Kind, Value, Row, Column)
	table.insert(State.Tokens, {
		Kind = Kind,
		Value = Value,
		Row = Row or State.File.Row,
		Column = Column or State.File.Column,
	})
end

function Tokenizer:IsWhitespace()
	return StringHasLetter(WhiteSpace, self.Character)
end

function Tokenizer:IsIdentifier()
	return StringHasLetter(BaseIdent, self.Character)
end

function Tokenizer:IsDigit()
	return StringHasLetter(BaseDigits, self.Character)
end

function Tokenizer:IsLetter()
	return StringHasLetter(BaseLetters .. "_", self.Character)
end

function Tokenizer:SkipComment(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column

	self:Advance(State)

	if self.Character ~= "-" then
		Logger:Error(State, "SyntaxError", "Unexpected character: '-'", StartRow, StartColumn)
		return false
	end

	self:Advance(State)

	if self.Character == "[" then
		self:Advance(State)

		if self.Character == "[" then
			self:Advance(State)

			while self.Character ~= "" do
				if self.Character == "]" then
					self:Advance(State)

					if self.Character == "]" then
						self:Advance(State)
						return true
					end
				else
					self:Advance(State)
				end
			end

			Logger:Error(State, "SyntaxError", "Unterminated multiline comment", StartRow, StartColumn)

			return false
		end
	end

	while self.Character ~= "" and self.Character ~= "\n" do
		self:Advance(State)
	end

	return true
end

function Tokenizer:ReadString(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column

	local Quote = self.Character

	self:Advance(State)

	local StartIndex = self.Index

	while self.Character ~= "" do
		if self.Character == "\\" then
			self:Advance(State)

			if self.Character ~= "" then
				self:Advance(State)
			end
		elseif self.Character == Quote then
			local Value = State.File.Content:sub(StartIndex, self.Index - 1)

			self:Advance(State)

			self:NewToken(State, "String", Value, StartRow, StartColumn)

			return true
		else
			self:Advance(State)
		end
	end

	Logger:Error(State, "SyntaxError", "Unterminated string", StartRow, StartColumn)

	return false
end

function Tokenizer:ReadIdentifier(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column
	local StartIndex = self.Index

	while self:IsIdentifier() do
		self:Advance(State)
	end

	local Value = State.File.Content:sub(StartIndex, self.Index - 1)

	local Kind = "Ident"

	if Value == "true" or Value == "false" then
		Kind = "Boolean"
	elseif Value == "nil" then
		Kind = "Nil"
	end

	self:NewToken(State, Kind, Value, StartRow, StartColumn)
end

function Tokenizer:ReadNumber(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column
	local StartIndex = self.Index

	while self:IsDigit() do
		self:Advance(State)
	end

	if self.Character == "." then
		self:Advance(State)

		while self:IsDigit() do
			self:Advance(State)
		end
	end

	local Value = State.File.Content:sub(StartIndex, self.Index - 1)

	self:NewToken(State, "Number", tonumber(Value), StartRow, StartColumn)
end

function Tokenizer:ReadExpression(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column

	self:Advance(State)

	local StartIndex = self.Index
	local Depth = 1

	while self.Character ~= "" do
		if self.Character == '"' or self.Character == "'" then
			local Quote = self.Character
			self:Advance(State)

			while self.Character ~= "" do
				if self.Character == "\\" then
					self:Advance(State)

					if self.Character ~= "" then
						self:Advance(State)
					end
				elseif self.Character == Quote then
					self:Advance(State)
					break
				else
					self:Advance(State)
				end
			end
		elseif self.Character == "-" then
			self:Advance(State)

			if self.Character == "-" then
				self:Advance(State)

				if self.Character == "[" then
					self:Advance(State)

					if self.Character == "[" then
						self:Advance(State)

						local Closed = false

						while self.Character ~= "" do
							if self.Character == "]" then
								self:Advance(State)

								if self.Character == "]" then
									self:Advance(State)
									Closed = true
									break
								end
							else
								self:Advance(State)
							end
						end

						if not Closed then
							Logger:Error(State, "SyntaxError", "Unterminated multiline comment", StartRow, StartColumn)
							return false
						end
					else
						while self.Character ~= "" and self.Character ~= "\n" do
							self:Advance(State)
						end
					end
				else
					while self.Character ~= "" and self.Character ~= "\n" do
						self:Advance(State)
					end
				end
			end
		elseif self.Character == "{" then
			Depth = Depth + 1
			self:Advance(State)
		elseif self.Character == "}" then
			Depth = Depth - 1

			if Depth == 0 then
				local Value = State.File.Content:sub(StartIndex, self.Index - 1)

				self:Advance(State)

				self:NewToken(State, "Expression", Value, StartRow, StartColumn)

				return true
			end

			self:Advance(State)
		else
			self:Advance(State)
		end
	end

	Logger:Error(State, "SyntaxError", "Unterminated expression", StartRow, StartColumn)

	return false
end

function Tokenizer:ReadText(State)
	local StartRow = State.File.Row
	local StartColumn = State.File.Column
	local StartIndex = self.Index

	while self.Character ~= "" and self.Character ~= "<" and self.Character ~= "{" do
		if self.Character == "-" then
			local NextCharacter = State.File.Content:sub(self.Index + 1, self.Index + 1)

			if NextCharacter == "-" then
				break
			end
		end

		self:Advance(State)
	end

	if self.Index > StartIndex then
		local Value = State.File.Content:sub(StartIndex, self.Index - 1)

		self:NewToken(State, "Text", Value, StartRow, StartColumn)
	end
end

function Tokenizer:Tokenize(State)
	self:Advance(State, 1)

	while self.Character ~= "" and not State.Stop do
		if self:IsWhitespace() then
			self:Advance(State)
		elseif self.Character == "-" then
			if not self:SkipComment(State) then
				break
			end
		elseif self.Character == "<" then
			local StartRow = State.File.Row
			local StartColumn = State.File.Column

			self:Advance(State)

			if self.Character == "/" then
				self:Advance(State)

				self:NewToken(State, "EndTagOpen", nil, StartRow, StartColumn)
			else
				self:NewToken(State, "TagOpen", nil, StartRow, StartColumn)
			end
		elseif self.Character == ">" then
			local StartRow = State.File.Row
			local StartColumn = State.File.Column

			self:Advance(State)

			self:NewToken(State, "TagClose", nil, StartRow, StartColumn)
		elseif self.Character == "/" then
			local StartRow = State.File.Row
			local StartColumn = State.File.Column

			self:Advance(State)

			if self.Character == ">" then
				self:Advance(State)

				self:NewToken(State, "EmptyTagClose", nil, StartRow, StartColumn)
			else
				Logger:Error(State, "SyntaxError", "Unexpected character: '/'", StartRow, StartColumn)
			end
		elseif self.Character == "=" then
			local StartRow = State.File.Row
			local StartColumn = State.File.Column

			self:Advance(State)

			self:NewToken(State, "Equal", "=", StartRow, StartColumn)
		elseif self.Character == '"' or self.Character == "'" then
			if not self:ReadString(State) then
				break
			end
		elseif self.Character == "{" then
			if not self:ReadExpression(State) then
				break
			end
		elseif self:IsLetter() then
			self:ReadIdentifier(State)
		elseif self:IsDigit() then
			self:ReadNumber(State)
		else
			self:ReadText(State)

			if self.Index == 0 then
				self:Advance(State)
			end
		end
	end

	self:NewToken(State, "EndOfFile")
end

local Parser = {}

function Parser:Reset(State)
	self.Index = 1
	self.Token = State.Tokens[1]
	self.Ast = {}

	State.Ast = {}
end

function Parser:Advance(State, Step)
	Step = Step or 1

	self.Index = self.Index + Step
	self.Token = State.Tokens[self.Index]
end

function Parser:Check(Kind, Value)
	if not self.Token then
		return false
	end

	if type(Kind) == "table" then
		local Found = false

		for _, Item in pairs(Kind) do
			if Item == self.Token.Kind then
				Found = true
				break
			end
		end

		if not Found then
			return false
		end
	elseif self.Token.Kind ~= Kind then
		return false
	end

	if Value ~= nil and self.Token.Value ~= Value then
		return false
	end

	return true
end

function Parser:Expect(State, Kind, Value)
	if not self:Check(Kind, Value) then
		local Token = self.Token or {}

		Logger:Error(
			State,
			"SyntaxError",
			string.format("Expected %s, got %s (%s)", tostring(Kind), tostring(Token.Kind), tostring(Token.Value)),
			Token.Row,
			Token.Column
		)

		return nil
	end

	local Token = self.Token
	self:Advance(State)

	return Token
end

function Parser:ParseValue(State)
	local Token = self.Token

	if self:Check({
		"String",
		"Number",
		"Boolean",
		"Nil",
		"Expression",
	}) then
		self:Advance(State)

		if Token.Kind == "Expression" then
			return {
				Kind = "Expression",
				Value = Token.Value,
			}
		end

		if Token.Kind == "Boolean" then
			return Token.Value == "true"
		end

		if Token.Kind == "Nil" then
			return nil
		end

		return Token.Value
	end

	if self:Check("Ident") then
		self:Advance(State)

		return {
			Kind = "Expression",
			Value = Token.Value,
		}
	end

	Logger:Error(State, "SyntaxError", "Expected an attribute value", Token and Token.Row, Token and Token.Column)

	return nil
end

function Parser:ParseAttributes(State)
	local Attributes = {}

	while self:Check("Ident") do
		local NameToken = self:Expect(State, "Ident")

		if not NameToken then
			return nil
		end

		local Name = NameToken.Value

		if Attributes[Name] ~= nil then
			Logger:Error(State, "SyntaxError", "Duplicate attribute: " .. Name, NameToken.Row, NameToken.Column)

			return nil
		end

		if not self:Expect(State, "Equal") then
			return nil
		end

		local Value = self:ParseValue(State)

		if State.Stop then
			return nil
		end

		Attributes[Name] = Value
	end

	return Attributes
end

function Parser:ParseElement(State)
	local OpenToken = self:Expect(State, "TagOpen")

	if not OpenToken then
		return nil
	end

	local NameToken = self:Expect(State, "Ident")

	if not NameToken then
		return nil
	end

	local Name = NameToken.Value

	local Attributes = self:ParseAttributes(State)

	if not Attributes then
		return nil
	end

	local Node = {
		Kind = "Element",
		Name = Name,
		Attributes = Attributes,
		Children = {},
	}

	if self:Check("EmptyTagClose") then
		self:Advance(State)
		return Node
	end

	if not self:Expect(State, "TagClose") then
		return nil
	end

	while self.Token and not self:Check("EndOfFile") do
		if self:Check("EndTagOpen") then
			self:Advance(State)

			local ClosingName = self:Expect(State, "Ident")

			if not ClosingName then
				return nil
			end

			if ClosingName.Value ~= Name then
				Logger:Error(
					State,
					"SyntaxError",
					string.format("Expected closing tag </%s>, got </%s>", Name, ClosingName.Value),
					ClosingName.Row,
					ClosingName.Column
				)

				return nil
			end

			if not self:Expect(State, "TagClose") then
				return nil
			end

			return Node
		elseif self:Check("TagOpen") then
			local Child = self:ParseElement(State)

			if not Child then
				return nil
			end

			table.insert(Node.Children, Child)
		elseif self:Check("Text") then
			local Token = self.Token
			self:Advance(State)

			table.insert(Node.Children, {
				Kind = "Text",
				Value = Token.Value,
			})
		elseif self:Check("Expression") then
			local Token = self.Token
			self:Advance(State)

			table.insert(Node.Children, {
				Kind = "Expression",
				Value = Token.Value,
			})
		else
			local Token = self.Token

			Logger:Error(State, "SyntaxError", "Unexpected token: " .. tostring(Token.Kind), Token.Row, Token.Column)

			return nil
		end
	end

	Logger:Error(State, "SyntaxError", "Unclosed element: <" .. Name .. ">", NameToken.Row, NameToken.Column)

	return nil
end

function Parser:Parse(State)
	self:Reset(State)

	while self.Token and not self:Check("EndOfFile") and not State.Stop do
		if self:Check("TagOpen") then
			local Node = self:ParseElement(State)

			if not Node then
				break
			end

			table.insert(self.Ast, Node)
		elseif self:Check("Text") then
			local Token = self.Token
			self:Advance(State)

			table.insert(self.Ast, {
				Kind = "Text",
				Value = Token.Value,
			})
		elseif self:Check("Expression") then
			local Token = self.Token
			self:Advance(State)

			table.insert(self.Ast, {
				Kind = "Expression",
				Value = Token.Value,
			})
		else
			local Token = self.Token

			Logger:Error(State, "SyntaxError", "Unexpected token: " .. tostring(Token.Kind), Token.Row, Token.Column)

			break
		end
	end

	State.Ast = self.Ast

	return not State.Stop
end

function MyaC.new()
	return {
		File = {
			Content = "",
			Name = "",
			Row = 1,
			Column = 1,
		},
		Tokens = {},
		Ast = {},
		Errors = {},
		Output = "",
		Stop = false,
	}
end

function MyaC:Tokenize(State)
	Tokenizer:Reset(State)
	Tokenizer:Tokenize(State)

	return not State.Stop
end

function MyaC:Parse(State)
	return Parser:Parse(State)
end

function MyaC:Compile(State, FileName, OutputFile)
	local FilePtr, Error = io.open(FileName, "r")

	if not FilePtr then
		return nil, Error
	end

	State.File.Content = FilePtr:read("*a")
	State.File.Name = FileName

	FilePtr:close()

	if not MyaC:Tokenize(State) then
		return nil, State.ErrorMessage
	end

	if not MyaC:Parse(State) then
		return nil, State.ErrorMessage
	end

	PrintTable(State.Ast)

	return State.Ast
end

return MyaC
