local CBackend = {}

local StyleSetters = {
  FontFamily = { "FontFamily", "sans-serif" },
  FontSize = { "FontSize", 16 },
  FontWeight = { "FontWeight", 400 },
  FontStyle = { "FontStyle", "normal" },
  LineHeight = { "LineHeight", "normal" },
  LetterSpacing = { "LetterSpacing", 0 },
  WordSpacing = { "WordSpacing", 0 },
  TextAlign = { "TextAlign", "start" },
  TextDecoration = { "TextDecoration", "none" },
  TextTransform = { "TextTransform", "none" },
  TextIndent = { "TextIndent", 0 },
  WhiteSpace = { "WhiteSpace", "normal" },
  WordBreak = { "WordBreak", "normal" },
  OverflowWrap = { "OverflowWrap", "normal" },
  Color = { "Color", "#000000" },
  BackgroundColor = { "BackgroundColor", "transparent" },
  Display = { "Display", "block" },
  Position = { "Position", "static" },
  Width = { "Width", "auto" },
  Height = { "Height", "auto" },
  MinWidth = { "MinWidth", "auto" },
  MinHeight = { "MinHeight", "auto" },
  MaxWidth = { "MaxWidth", "none" },
  MaxHeight = { "MaxHeight", "none" },
  MarginTop = { "MarginTop", 0 },
  MarginRight = { "MarginRight", 0 },
  MarginBottom = { "MarginBottom", 0 },
  MarginLeft = { "MarginLeft", 0 },
  PaddingTop = { "PaddingTop", 0 },
  PaddingRight = { "PaddingRight", 0 },
  PaddingBottom = { "PaddingBottom", 0 },
  PaddingLeft = { "PaddingLeft", 0 },
  BorderWidth = { "BorderWidth", 0 },
  BorderColor = { "BorderColor", "#000000" },
  BorderStyle = { "BorderStyle", "none" },
  BorderRadius = { "BorderRadius", 0 },
  FlexDirection = { "FlexDirection", "row" },
  FlexWrap = { "FlexWrap", "nowrap" },
  JustifyContent = { "JustifyContent", "normal" },
  AlignItems = { "AlignItems", "normal" },
  AlignContent = { "AlignContent", "normal" },
  AlignSelf = { "AlignSelf", "auto" },
  FlexGrow = { "FlexGrow", 0 },
  FlexShrink = { "FlexShrink", 1 },
  FlexBasis = { "FlexBasis", "auto" },
  Gap = { "Gap", 0 },
  RowGap = { "RowGap", 0 },
  ColumnGap = { "ColumnGap", 0 },
  Overflow = { "Overflow", "visible" },
  OverflowX = { "OverflowX", "visible" },
  OverflowY = { "OverflowY", "visible" },
  Opacity = { "Opacity", 1 },
  Visibility = { "Visibility", "visible" },
  ZIndex = { "ZIndex", "auto" },
  Cursor = { "Cursor", "auto" },
}

function CBackend:PutFormatted(State, Format, ...)
  local Output = string.format(Format, ...)

  Output = Output:gsub("\n([^\n])", "\n" .. "%1")

  self.CCode = self.CCode .. Output
end

function CBackend:GetStyleFromAst(Ast, ParentStyle)
  ParentStyle = ParentStyle or {}

  local Inherited = {
    FontFamily = true,
    FontSize = true,
    FontWeight = true,
    FontStyle = true,
    Color = true,
    Visibility = true,
  }

  local Style = {}

  for Property, Info in pairs(StyleSetters) do
    local Default = Info[2]
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

function CBackend:GenerateStyles(State, ElementId, Style)
  for Property, Info in pairs(StyleSetters) do
    local Setter = Info[1]
    local Default = Info[2]
    local Value = Style[Property]

    if Value ~= nil and Value ~= Default then
      self:PutFormatted(State, "MyaElement_Set%s(Element%s, %q);\n", Setter, tostring(ElementId), tostring(Value))
    end
  end
end

function CBackend:Generate(State, Ast, Parent, ParentStyle)
  ParentStyle = ParentStyle or {}

  local Style = self:GetStyleFromAst(Ast, ParentStyle)

  if Ast.Kind == "Element" then
    self:PutFormatted(State, "MyaElement *Element%s = MyaElement_Create(%q);\n", Ast.Id, Ast.TagName)
    self:GenerateStyles(State, Ast.Id, Style)
    if Parent then
      self:PutFormatted(State, "MyaElement_AppendChild(Element%s, Element%s);\n", Parent.Id, Ast.Id)
    end

    for Key, Value in pairs(Ast.Attributes) do
      self:PutFormatted(State, "MyaElement_SetAttribute(Element%d, %q, %q);\n", Ast.Id, Key, Value)
    end

    for _, Child in ipairs(Ast.Children) do
      self:Generate(State, Child, Ast, Style)
    end
  elseif Ast.Kind == "Text" then
    self:PutFormatted(State, "MyaElement_SetText(Element%d, %q);\n", Parent.Id, Ast.Content)
  elseif Ast.Kind == "Comment" then
  elseif Ast.Kind == "Doctype" then
  elseif Ast.Kind == "Document" then
  end
end

function CBackend:Compile(State)
  self.CCode = ""

  self:PutFormatted(State, 'MyaElement *Document = MyaElement_Create("document");\n')

  for _, Ast in ipairs(State.Ast) do
    self:Generate(State, Ast, {
      Id = "Document",
    })
  end

  self.Output = self.CCode
  return self.CCode
end

return CBackend
