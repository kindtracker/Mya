local CBackend = {}

function CBackend:PutFormatted(State, Format, ...)
  CBackend.CCode = CBackend.CCode .. string.format(Format, ...)
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

      for _, Child in ipairs(Ast.Children) do
        self:Generate(State, Child)
      end

      self:PutFormatted(State, "}\n")
    end
  elseif Ast.Kind == "Text" then
    self:PutFormatted(State, "CLAY_TEXT(CLAY_STRING(%q), CLAY_TEXT_CONFIG({ .fontSize = 32 }));\n", Ast.Content)
  end
end

function CBackend:Compile(State)
  self.CCode = ""

  for _, Ast in ipairs(State.Ast) do
    self:Generate(State, Ast)
  end

  self.Output = self.CCode
  return self.CCode
end

return CBackend
