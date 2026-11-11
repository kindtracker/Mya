local CGenerator = {}

function CGenerator:PutFormatted(State, Format, ...)
  local Output = string.format(Format, ...)

  Output = Output:gsub("\n([^\n])", "\n" .. "%1")

  self.CCode = self.CCode .. Output
end

function CGenerator:Generate(State, Ast, Parent)
  if Ast.Kind == "Element" then
    self:PutFormatted(State, "  MyaElement *%s = MyaCreate(%q);\n", Ast.Id, Ast.TagName)
    if Parent then
      self:PutFormatted(State, "  MyaAppendChild(%s, %s);\n", Parent.Id, Ast.Id)
    end

    for Key, Value in pairs(Ast.Attributes) do
      self:PutFormatted(State, "  MyaSetAttribute(%s, %q, %q);\n", Ast.Id, Key, Value)
    end

    for _, Child in ipairs(Ast.Children) do
      self:Generate(State, Child, Ast)
    end
  elseif Ast.Kind == "Text" then
    self:PutFormatted(State, "  MyaSetText(%s, %q);\n", Parent.Id, Ast.Content)
  elseif Ast.Kind == "Comment" then
  elseif Ast.Kind == "Doctype" then
  elseif Ast.Kind == "Document" then
  end
end

function CGenerator:Compile(State)
  self.CCode = ""

  self:PutFormatted(
    State,
    [[
#include "Mya.h"

static MyaElement *__Mya_Document;

MyaElement *MyaCreatePage() {
  __Mya_Document = MyaCreate("document");
]]
  )

  for _, Ast in ipairs(State.Ast) do
    self:Generate(State, Ast, {
      Id = "__Mya_Document",
    })
  end

  self:PutFormatted(
    State,
    [[

  return __Mya_Document;
}
]]
  )

  self.Output = self.CCode
  return self.CCode
end

return CGenerator
