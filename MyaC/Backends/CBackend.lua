local CBackend = {}

function CBackend:PutFormatted(State, Format, ...)
  local Output = string.format(Format, ...)

  Output = Output:gsub("\n([^\n])", "\n" .. string.rep("\t", CBackend.Indent) .. "%1")

  if self.CCode == "" or self.CCode:sub(-1) == "\n" then
    Output = string.rep("\t", CBackend.Indent) .. Output
  end

  self.CCode = self.CCode .. Output
end

function CBackend:GetStyleFromAst(Ast, ParentStyle)
  ParentStyle = ParentStyle or {}

  local Defaults = {
    FontFamily = "sans-serif",
    FontSize = 16,
    FontWeight = 400,
    FontStyle = "normal",
    LineHeight = "normal",
    LetterSpacing = 0,
    WordSpacing = 0,
    TextAlign = "start",
    TextDecoration = "none",
    TextTransform = "none",
    TextIndent = 0,
    WhiteSpace = "normal",
    WordBreak = "normal",
    OverflowWrap = "normal",
    Color = "#000000",
    BackgroundColor = "transparent",
    Display = "block",
    Position = "static",
    Width = "auto",
    Height = "auto",
    MinWidth = "auto",
    MinHeight = "auto",
    MaxWidth = "none",
    MaxHeight = "none",
    MarginTop = 0,
    MarginRight = 0,
    MarginBottom = 0,
    MarginLeft = 0,
    PaddingTop = 0,
    PaddingRight = 0,
    PaddingBottom = 0,
    PaddingLeft = 0,
    BorderWidth = 0,
    BorderColor = "#000000",
    BorderStyle = "none",
    BorderRadius = 0,
    FlexDirection = "row",
    FlexWrap = "nowrap",
    JustifyContent = "normal",
    AlignItems = "normal",
    AlignContent = "normal",
    AlignSelf = "auto",
    FlexGrow = 0,
    FlexShrink = 1,
    FlexBasis = "auto",
    Gap = 0,
    RowGap = 0,
    ColumnGap = 0,
    Overflow = "visible",
    OverflowX = "visible",
    OverflowY = "visible",
    Opacity = 1,
    Visibility = "visible",
    ZIndex = "auto",
    Cursor = "auto",
  }

  local Inherited = {
    FontFamily = true,
    FontSize = true,
    FontWeight = true,
    FontStyle = true,
    LineHeight = true,
    LetterSpacing = true,
    WordSpacing = true,
    TextAlign = true,
    TextDecoration = true,
    TextTransform = true,
    TextIndent = true,
    WhiteSpace = true,
    WordBreak = true,
    OverflowWrap = true,
    Color = true,
    Cursor = true,
    Visibility = true,
  }

  local Style = {}

  for Property, Default in pairs(Defaults) do
    local Value = Ast[Property]

    if Value ~= nil then
      Style[Property] = Value
    elseif Inherited[Property] and ParentStyle[Property] ~= nil then
      Style[Property] = ParentStyle[Property]
    else
      Style[Property] = Default
    end
  end

  return Style
end

function CBackend:Generate(State, Ast, ParentStyle)
  ParentStyle = ParentStyle or {}

  local Style = self:GetStyleFromAst(Ast, ParentStyle)
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

      self.Indent = self.Indent + 1
      for _, Child in ipairs(Ast.Children) do
        self:Generate(State, Child, Style)
      end
      self.Indent = self.Indent - 1
      self:PutFormatted(State, "}\n")
    end
  elseif Ast.Kind == "Text" then
    self:PutFormatted(
      State,
      "CLAY_TEXT(CLAY_STRING(%q), CLAY_TEXT_CONFIG({ .fontSize = %d }));\n",
      Ast.Content,
      Style.FontSize
    )
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
