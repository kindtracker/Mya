local CBackend = {}

function CBackend:PutFormatted(State, Format, ...)
  local Output = string.format(Format, ...)

  Output = Output:gsub("\n([^\n])", "\n" .. string.rep("\t", CBackend.Indent) .. "%1")

  if self.CCode == "" or self.CCode:sub(-1) == "\n" then
    Output = string.rep("\t", CBackend.Indent) .. Output
  end

  self.CCode = self.CCode .. Output
end

function CBackend:Generate(State, Ast)
  if Ast.Kind == "Element" then
    if Ast.TagName == "h1" then
      self:PutFormatted(
        State,
        [[
CLAY({
	.layout = {
		.sizing = {
			.width = CLAY_SIZING_FIT,
			.height = CLAY_SIZING_FIT
		}
	}
}) {
]]
      )

      CBackend.Indent = CBackend.Indent + 1
      for _, Child in ipairs(Ast.Children) do
        self:Generate(State, Child)
      end
      CBackend.Indent = CBackend.Indent - 1

      self:PutFormatted(State, "}\n")
    end
  elseif Ast.Kind == "Text" then
    self:PutFormatted(State, "CLAY_TEXT(CLAY_STRING(%q), CLAY_TEXT_CONFIG({ .fontSize = 32 }));\n", Ast.Content)
  end
end

function CBackend:Compile(State)
  self.CCode = ""

  CBackend.Indent = 0
  for _, Ast in ipairs(State.Ast) do
    self:Generate(State, Ast)
  end

  self.Output = self.CCode
  return self.CCode
end

return CBackend
