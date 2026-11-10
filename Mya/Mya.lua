local Mya = {}

local VoidElements = {
  area = true,
  base = true,
  br = true,
  col = true,
  embed = true,
  hr = true,
  img = true,
  input = true,
  link = true,
  meta = true,
  param = true,
  source = true,
  track = true,
  wbr = true,
}

local RawTextElements = {
  script = true,
  style = true,
}

local RCDATAElements = {
  title = true,
  textarea = true,
}

local BlockElements = {
  address = true,
  article = true,
  aside = true,
  blockquote = true,
  div = true,
  dl = true,
  fieldset = true,
  footer = true,
  form = true,
  h1 = true,
  h2 = true,
  h3 = true,
  h4 = true,
  h5 = true,
  h6 = true,
  header = true,
  hr = true,
  main = true,
  nav = true,
  ol = true,
  p = true,
  pre = true,
  section = true,
  table = true,
  ul = true,
}

local EntityMap = {
  amp = "&",
  apos = "'",
  gt = ">",
  lt = "<",
  quot = '"',
  nbsp = "\194\160",
  copy = "\194\169",
  reg = "\194\174",
  trade = "\226\132\162",
  mdash = "\226\128\148",
  ndash = "\226\128\147",
  hellip = "\226\128\166",
  lsquo = "\226\128\152",
  rsquo = "\226\128\153",
  ldquo = "\226\128\156",
  rdquo = "\226\128\157",
  euro = "\226\130\172",
}

local Ansi = {
  Reset = "\27[0m",
  Red = "\27[31m",
  RuaRed = "\27[38;2;255;100;100m",
  LuaBlue = "\27[38;2;100;200;255m",
}

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

local function EncodeUTF8(Codepoint)
  if Codepoint <= 0x7F then
    return string.char(Codepoint)
  elseif Codepoint <= 0x7FF then
    return string.char(0xC0 + math.floor(Codepoint / 64), 0x80 + Codepoint % 64)
  elseif Codepoint <= 0xFFFF then
    return string.char(
      0xE0 + math.floor(Codepoint / 4096),
      0x80 + math.floor(Codepoint / 64) % 64,
      0x80 + Codepoint % 64
    )
  elseif Codepoint <= 0x10FFFF then
    return string.char(
      0xF0 + math.floor(Codepoint / 262144),
      0x80 + math.floor(Codepoint / 4096) % 64,
      0x80 + math.floor(Codepoint / 64) % 64,
      0x80 + Codepoint % 64
    )
  end

  return "\239\191\189"
end

local function DecodeEntities(Value)
  return (
    Value:gsub("&(#?x?[%w]+);?", function(Entity)
      local Named = Entity:match("^&([%a]+);?$")

      if Named then
        return EntityMap[Named] or "&" .. Entity:sub(2)
      end

      local Hex = Entity:match("^&#[xX]([%da-fA-F]+);?$")
      local Decimal = Entity:match("^&#(%d+);?$")

      local Codepoint

      if Hex then
        Codepoint = tonumber(Hex, 16)
      elseif Decimal then
        Codepoint = tonumber(Decimal)
      else
        return "&" .. Entity:sub(2)
      end

      if not Codepoint or Codepoint == 0 or Codepoint > 0x10FFFF then
        return "\239\191\189"
      end

      if Codepoint >= 0xD800 and Codepoint <= 0xDFFF then
        return "\239\191\189"
      end

      local Replacements = {
        [0x80] = 0x20AC,
        [0x82] = 0x201A,
        [0x83] = 0x0192,
        [0x84] = 0x201E,
        [0x85] = 0x2026,
        [0x86] = 0x2020,
        [0x87] = 0x2021,
        [0x88] = 0x02C6,
        [0x89] = 0x2030,
        [0x8A] = 0x0160,
        [0x8B] = 0x2039,
        [0x8C] = 0x0152,
        [0x8E] = 0x017D,
        [0x91] = 0x2018,
        [0x92] = 0x2019,
        [0x93] = 0x201C,
        [0x94] = 0x201D,
        [0x95] = 0x2022,
        [0x96] = 0x2013,
        [0x97] = 0x2014,
        [0x98] = 0x02DC,
        [0x99] = 0x2122,
        [0x9A] = 0x0161,
        [0x9B] = 0x203A,
        [0x9C] = 0x0153,
        [0x9E] = 0x017E,
        [0x9F] = 0x0178,
      }

      Codepoint = Replacements[Codepoint] or Codepoint

      return EncodeUTF8(Codepoint)
    end)
  )
end

local function IsWhitespace(Character)
  return Character == " " or Character == "\t" or Character == "\n" or Character == "\r" or Character == "\f"
end

local function IsNameCharacter(Character)
  return Character ~= "" and not IsWhitespace(Character) and Character ~= "/" and Character ~= ">" and Character ~= "="
end

local function FindClosingTag(Content, Name, Start)
  local LowerContent = Content:lower()
  local Pattern = "</%s*" .. Name .. "%s*[%/>]"

  local Position = LowerContent:find(Pattern, Start)

  return Position
end

local Logger = {}

function Logger:Error(State, ErrorType, ErrorMessage, Row, Column)
  Row = Row or State.File.Row
  Column = Column or State.File.Column

  local Error = {
    Type = ErrorType,
    Message = ErrorMessage,
    Row = Row,
    Column = Column,
  }

  table.insert(State.Errors, Error)

  State.ErrorMessage = ErrorMessage
  State.Stop = true

  io.write(
    string.format(
      "%sMya:%s %s:%d:%d: %sError:%s %s: %s\n",
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

  if Mya._DevMode then
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
  self.Content = State.File.Content
  self.Length = #self.Content

  self.Index = 1
  self.Row = 1
  self.Column = 1

  self.Character = self.Content:sub(1, 1)
  self.RawTag = nil

  State.Tokens = {}
  State.Errors = {}
  State.Stop = false

  State.File.Row = 1
  State.File.Column = 1
end

function Tokenizer:Advance(State, Step)
  Step = Step or 1

  for _ = 1, Step do
    if self.Index > self.Length then
      self.Character = ""
      return
    end

    if self.Character == "\n" then
      self.Row = self.Row + 1
      self.Column = 1
    else
      self.Column = self.Column + 1
    end

    self.Index = self.Index + 1
    self.Character = self.Content:sub(self.Index, self.Index)
  end

  State.File.Row = self.Row
  State.File.Column = self.Column
end

function Tokenizer:NewToken(State, Kind, Value, Row, Column)
  table.insert(State.Tokens, {
    Kind = Kind,
    Value = Value,
    Row = Row or self.Row,
    Column = Column or self.Column,
  })
end

function Tokenizer:StartsWith(Value)
  return self.Content:sub(self.Index, self.Index + #Value - 1) == Value
end

function Tokenizer:SkipWhitespace(State)
  while self.Character ~= "" and IsWhitespace(self.Character) do
    self:Advance(State)
  end
end

function Tokenizer:ReadText(State)
  local Start = self.Index
  local Row = self.Row
  local Column = self.Column

  while self.Character ~= "" and self.Character ~= "<" do
    self:Advance(State)
  end

  if self.Index > Start then
    self:NewToken(State, "Text", self.Content:sub(Start, self.Index - 1), Row, Column)
  end
end

function Tokenizer:ReadComment(State)
  local Row = self.Row
  local Column = self.Column

  self:Advance(State, 4)

  local Start = self.Index

  local Position = self.Content:find("-->", self.Index, true)

  if not Position then
    local Value = self.Content:sub(Start)

    self:Advance(State, self.Length - self.Index + 1)

    self:NewToken(State, "Comment", Value, Row, Column)
    return
  end

  local Value = self.Content:sub(Start, Position - 1)

  self:Advance(State, Position + 3 - self.Index)

  self:NewToken(State, "Comment", Value, Row, Column)
end

function Tokenizer:ReadDoctype(State)
  local Row = self.Row
  local Column = self.Column

  self:Advance(State, 9)

  local Start = self.Index
  local Quote = nil

  while self.Character ~= "" do
    if Quote then
      if self.Character == Quote then
        Quote = nil
      end
    elseif self.Character == '"' or self.Character == "'" then
      Quote = self.Character
    elseif self.Character == ">" then
      break
    end

    self:Advance(State)
  end

  local Value = self.Content:sub(Start, self.Index - 1):match("^%s*(.-)%s*$")

  if self.Character == ">" then
    self:Advance(State)
  end

  self:NewToken(State, "Doctype", Value, Row, Column)
end

function Tokenizer:ReadTag(State)
  local Row = self.Row
  local Column = self.Column

  self:Advance(State)

  local Closing = false

  if self.Character == "/" then
    Closing = true
    self:Advance(State)
  end

  if not self.Character:match("[%a:]") then
    local Start = self.Index

    while self.Character ~= "" and self.Character ~= ">" do
      self:Advance(State)
    end

    local Value = self.Content:sub(Start, self.Index - 1)

    if self.Character == ">" then
      self:Advance(State)
    end

    self:NewToken(State, "Comment", Value, Row, Column)
    return
  end

  local Start = self.Index

  while IsNameCharacter(self.Character) do
    self:Advance(State)
  end

  local Name = self.Content:sub(Start, self.Index - 1):lower()

  if Closing then
    self:SkipWhitespace(State)

    if self.Character == ">" then
      self:Advance(State)
    else
      while self.Character ~= "" and self.Character ~= ">" do
        self:Advance(State)
      end

      if self.Character == ">" then
        self:Advance(State)
      end
    end

    self:NewToken(State, "EndTag", Name, Row, Column)
    return
  end

  local Attributes = {}
  local SelfClosing = false

  while self.Character ~= "" do
    self:SkipWhitespace(State)

    if self.Character == ">" then
      self:Advance(State)
      break
    elseif self.Character == "/" then
      self:Advance(State)

      if self.Character == ">" then
        SelfClosing = true
        self:Advance(State)
        break
      end
    else
      local AttrStart = self.Index

      while
        self.Character ~= ""
        and not IsWhitespace(self.Character)
        and self.Character ~= "="
        and self.Character ~= ">"
        and self.Character ~= "/"
      do
        self:Advance(State)
      end

      if self.Index == AttrStart then
        self:Advance(State)
      else
        local AttrName = self.Content:sub(AttrStart, self.Index - 1):lower()

        self:SkipWhitespace(State)

        local AttrValue = true

        if self.Character == "=" then
          self:Advance(State)
          self:SkipWhitespace(State)

          if self.Character == '"' or self.Character == "'" then
            local Quote = self.Character
            self:Advance(State)

            local ValueStart = self.Index

            while self.Character ~= "" and self.Character ~= Quote do
              self:Advance(State)
            end

            AttrValue = self.Content:sub(ValueStart, self.Index - 1)

            if self.Character == Quote then
              self:Advance(State)
            end
          else
            local ValueStart = self.Index

            while self.Character ~= "" and not IsWhitespace(self.Character) and self.Character ~= ">" do
              self:Advance(State)
            end

            AttrValue = self.Content:sub(ValueStart, self.Index - 1)
          end
        end

        if Attributes[AttrName] == nil then
          Attributes[AttrName] = AttrValue
        end
      end
    end
  end

  for Key, Value in pairs(Attributes) do
    if type(Value) == "string" then
      Attributes[Key] = DecodeEntities(Value)
    end
  end

  self:NewToken(State, "StartTag", {
    Name = Name,
    Attributes = Attributes,
    SelfClosing = SelfClosing,
  }, Row, Column)

  if RawTextElements[Name] or RCDATAElements[Name] then
    self.RawTag = Name
  end
end

function Tokenizer:ReadRawText(State)
  local Name = self.RawTag
  local Start = self.Index
  local Row = self.Row
  local Column = self.Column

  local Position = FindClosingTag(self.Content, Name, self.Index)

  if not Position then
    local Value = self.Content:sub(Start)

    self:Advance(State, self.Length - self.Index + 1)

    self.RawTag = nil

    if Value ~= "" then
      if RCDATAElements[Name] then
        Value = DecodeEntities(Value)
      end

      self:NewToken(State, "Text", Value, Row, Column)
    end

    return
  end

  local Value = self.Content:sub(Start, Position - 1)

  self:Advance(State, Position - self.Index)

  self.RawTag = nil

  if Value ~= "" then
    if RCDATAElements[Name] then
      Value = DecodeEntities(Value)
    end

    self:NewToken(State, "Text", Value, Row, Column)
  end
end

function Tokenizer:Tokenize(State)
  while self.Character ~= "" and not State.Stop do
    if self.RawTag then
      self:ReadRawText(State)
    elseif self:StartsWith("<!--") then
      self:ReadComment(State)
    elseif self.Content:sub(self.Index, self.Index + 8):lower() == "<!doctype" then
      self:ReadDoctype(State)
    elseif self:StartsWith("</") or self:StartsWith("<") then
      self:ReadTag(State)
    else
      self:ReadText(State)
    end
  end

  self:NewToken(State, "EndOfFile", nil)
end

local Parser = {}

function Parser:Reset(State)
  self.Index = 1
  self.Token = State.Tokens[1]

  self.Ast = {}

  self.Stack = {
    {
      Kind = "Document",
      Children = self.Ast,
    },
  }

  State.Ast = self.Ast
end

function Parser:Advance(State)
  self.Index = self.Index + 1
  self.Token = State.Tokens[self.Index]
end

function Parser:Check(Kind)
  return self.Token and self.Token.Kind == Kind
end

function Parser:Current()
  return self.Stack[#self.Stack]
end

function Parser:Append(Node)
  local Parent = self:Current()

  local Characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"
  local IdTable = {}

  for Idx = 1, 16 do
    local RandomIndex = math.random(#Characters)
    IdTable[Idx] = Characters:sub(RandomIndex, RandomIndex)
  end

  local Id = "__Mya_" .. table.concat(IdTable)
  Node.Id = Id

  table.insert(Parent.Children, Node)
end

function Parser:FindOpen(Name)
  for Index = #self.Stack, 2, -1 do
    if self.Stack[Index].Name == Name then
      return Index
    end
  end

  return nil
end

function Parser:Close(Name)
  local Index = self:FindOpen(Name)

  if Index then
    while #self.Stack >= Index do
      table.remove(self.Stack)
    end
  end
end

function Parser:HasOpen(Name)
  return self:FindOpen(Name) ~= nil
end

function Parser:AutoClose(Name)
  local Current = self:Current()

  if Name == "p" and self:HasOpen("p") then
    self:Close("p")
  end

  if BlockElements[Name] and self:HasOpen("p") then
    self:Close("p")
  end

  if Name == "li" and self:HasOpen("li") then
    self:Close("li")
  end

  if (Name == "dt" or Name == "dd") and (self:HasOpen("dt") or self:HasOpen("dd")) then
    if self:HasOpen("dt") then
      self:Close("dt")
    else
      self:Close("dd")
    end
  end

  if Name:match("^h[1-6]$") then
    for Index = 1, 6 do
      if self:HasOpen("h" .. Index) then
        self:Close("h" .. Index)
        break
      end
    end
  end

  if Name == "tr" and self:HasOpen("tr") then
    self:Close("tr")
  end

  if Name == "td" or Name == "th" then
    if self:HasOpen("td") then
      self:Close("td")
    elseif self:HasOpen("th") then
      self:Close("th")
    end
  end

  if Name == "option" and self:HasOpen("option") then
    self:Close("option")
  end

  if Name == "optgroup" then
    if self:HasOpen("option") then
      self:Close("option")
    end

    if self:HasOpen("optgroup") then
      self:Close("optgroup")
    end
  end

  if Name == "rt" or Name == "rp" then
    if self:HasOpen("rt") then
      self:Close("rt")
    elseif self:HasOpen("rp") then
      self:Close("rp")
    end
  end

  if Name == "body" and self:HasOpen("head") then
    self:Close("head")
  end

  if Name == "head" and self:HasOpen("head") then
    self:Close("head")
  end

  if Name == "tbody" or Name == "thead" or Name == "tfoot" then
    if self:HasOpen("tbody") then
      self:Close("tbody")
    elseif self:HasOpen("thead") then
      self:Close("thead")
    elseif self:HasOpen("tfoot") then
      self:Close("tfoot")
    end
  end
end

function Parser:ParseStartTag(State)
  local Token = self.Token
  local Data = Token.Value

  self:AutoClose(Data.Name)

  local Node = {
    Kind = "Element",
    TagName = Data.Name,
    Attributes = Data.Attributes,
    Children = {},
    Row = Token.Row,
    Column = Token.Column,
  }

  self:Append(Node)

  if not VoidElements[Data.Name] and not Data.SelfClosing then
    table.insert(self.Stack, Node)
  end

  self:Advance(State)
end

function Parser:ParseEndTag(State)
  local Token = self.Token
  local Name = Token.Value

  local Index = self:FindOpen(Name)

  if Index then
    while #self.Stack >= Index do
      table.remove(self.Stack)
    end
  end

  self:Advance(State)
end

function Parser:ParseText(State)
  local Token = self.Token

  local Value = Token.Value

  local Parent = self:Current()

  if not (Parent.Name and RawTextElements[Parent.Name]) then
    Value = DecodeEntities(Value)
  end

  local IsTextWhitespace = true
  for Index = 1, #Value do
    local Character = Value:sub(Index, Index)

    if not IsWhitespace(Character) then
      IsTextWhitespace = false
      break
    end
  end

  if Value ~= "" and not IsTextWhitespace then
    self:Append({
      Kind = "Text",
      Content = Value,
      Row = Token.Row,
      Column = Token.Column,
    })
  end

  self:Advance(State)
end

function Parser:Parse(State)
  self:Reset(State)

  while self.Token and not self:Check("EndOfFile") do
    if self:Check("StartTag") then
      self:ParseStartTag(State)
    elseif self:Check("EndTag") then
      self:ParseEndTag(State)
    elseif self:Check("Text") then
      self:ParseText(State)
    elseif self:Check("Comment") then
      local Token = self.Token

      self:Append({
        Kind = "Comment",
        Value = Token.Value,
        Row = Token.Row,
        Column = Token.Column,
      })

      self:Advance(State)
    elseif self:Check("Doctype") then
      local Token = self.Token

      self:Append({
        Kind = "Doctype",
        Value = Token.Value,
        Row = Token.Row,
        Column = Token.Column,
      })

      self:Advance(State)
    else
      self:Advance(State)
    end
  end

  State.Ast = self.Ast

  return true
end

function Mya.new()
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
    ErrorMessage = nil,
    Stop = false,
  }
end

function Mya:Tokenize(State)
  Tokenizer:Reset(State)
  Tokenizer:Tokenize(State)

  return not State.Stop
end

function Mya:Parse(State)
  Parser:Reset(State)
  return Parser:Parse(State)
end

function Mya:UseBackend(State, Backend)
  State.Backend = Backend
end

function Mya:Compile(State, FileName, OutputFile)
  local FilePtr, Error = io.open(FileName, "r")

  if not FilePtr then
    return nil, Error
  end

  State.File.Content = FilePtr:read("*a")
  State.File.Name = FileName

  FilePtr:close()

  if not self:Tokenize(State) then
    return nil, State.ErrorMessage
  end

  if not self:Parse(State) then
    return nil, State.ErrorMessage
  end

  State.Backend:Compile(State)

  if OutputFile then
    local OutputPtr = io.open(OutputFile, "w")
    OutputPtr:write(State.Backend.Output)
    OutputPtr:close()
  end

  return State.Backend.Output
end

return Mya
