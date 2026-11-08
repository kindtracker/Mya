#include "Mya.h"

#include <stdlib.h>
#include <string.h>

static char *MyaStrdup(const char *String) {
  if (!String) {
    String = "";
  }

  size_t Length = strlen(String) + 1;
  char *Copy = malloc(Length);
  if (!Copy) {
    return NULL;
  }

  memcpy(Copy, String, Length);
  return Copy;
}

static void MyaSetEntry(MyaEntry *Entries, const char *Name,
                        const char *Value) {
  MyaEntry *Current = Entries;

  while (Current) {
    if (strcmp(Name, Current->Name) == 0) {
      char *Copy = MyaStrdup(Value);
      if (!Copy)
        return;

      free(Current->Value);
      Current->Value = Copy;
      return;
    }

    if (!Current->Next) {
      break;
    }

    Current = Current->Next;
  }

  if (!Current) {
    return;
  }

  MyaEntry *NewEntry = malloc(sizeof(MyaEntry));
  if (!NewEntry) {
    return;
  }

  NewEntry->Name = MyaStrdup(Name);
  NewEntry->Value = MyaStrdup(Value);

  if (!NewEntry->Name || !NewEntry->Value) {
    free(NewEntry->Name);
    free(NewEntry->Value);
    free(NewEntry);
    return;
  }

  NewEntry->Next = NULL;
  Current->Next = NewEntry;
}

#define MYA_STYLE(Name, Value) MyaSetEntry(Element->Styles, #Name, Value)
MyaElement *MyaCreate(const char *TagName) {
  MyaElement *Element = calloc(1, sizeof(MyaElement));
  if (!Element) {
    return NULL;
  }

  Element->TagName = MyaStrdup(TagName);
  if (!Element->TagName) {
    free(Element);
    return NULL;
  }

  MYA_STYLE(FontFamily, "sans-serif");
  MYA_STYLE(FontSize, "16");
  MYA_STYLE(FontWeight, "400");
  MYA_STYLE(FontStyle, "normal");
  MYA_STYLE(LineHeight, "normal");
  MYA_STYLE(LetterSpacing, "0");
  MYA_STYLE(WordSpacing, "0");
  MYA_STYLE(TextAlign, "start");
  MYA_STYLE(TextDecoration, "none");
  MYA_STYLE(TextTransform, "none");
  MYA_STYLE(TextIndent, "0");
  MYA_STYLE(WhiteSpace, "normal");
  MYA_STYLE(WordBreak, "normal");
  MYA_STYLE(OverflowWrap, "normal");
  MYA_STYLE(Color, "#000000");
  MYA_STYLE(BackgroundColor, "transparent");
  MYA_STYLE(Display, "block");
  MYA_STYLE(Position, "static");
  MYA_STYLE(Width, "auto");
  MYA_STYLE(Height, "auto");
  MYA_STYLE(MinWidth, "auto");
  MYA_STYLE(MinHeight, "auto");
  MYA_STYLE(MaxWidth, "none");
  MYA_STYLE(MaxHeight, "none");
  MYA_STYLE(MarginTop, "0");
  MYA_STYLE(MarginRight, "0");
  MYA_STYLE(MarginBottom, "0");
  MYA_STYLE(MarginLeft, "0");
  MYA_STYLE(PaddingTop, "0");
  MYA_STYLE(PaddingRight, "0");
  MYA_STYLE(PaddingBottom, "0");
  MYA_STYLE(PaddingLeft, "0");
  MYA_STYLE(BorderWidth, "0");
  MYA_STYLE(BorderColor, "#000000");
  MYA_STYLE(BorderStyle, "none");
  MYA_STYLE(BorderRadius, "0");
  MYA_STYLE(FlexDirection, "row");
  MYA_STYLE(FlexWrap, "nowrap");
  MYA_STYLE(JustifyContent, "normal");
  MYA_STYLE(AlignItems, "normal");
  MYA_STYLE(AlignContent, "normal");
  MYA_STYLE(AlignSelf, "auto");
  MYA_STYLE(FlexGrow, "0");
  MYA_STYLE(FlexShrink, "1");
  MYA_STYLE(FlexBasis, "auto");
  MYA_STYLE(Gap, "0");
  MYA_STYLE(RowGap, "0");
  MYA_STYLE(ColumnGap, "0");
  MYA_STYLE(Overflow, "visible");
  MYA_STYLE(OverflowX, "visible");
  MYA_STYLE(OverflowY, "visible");
  MYA_STYLE(Opacity, "1");
  MYA_STYLE(Visibility, "visible");
  MYA_STYLE(ZIndex, "auto");
  MYA_STYLE(Cursor, "auto");

  return Element;
}
#undef MYA_STYLE

static void MyaDetach(MyaElement *Element) {
  if (!Element || !Element->Parent)
    return;

  MyaElement *Parent = Element->Parent;
  MyaElement *Previous = NULL;
  MyaElement *Current = Parent->FirstChild;

  while (Current) {
    if (Current == Element) {
      if (Previous)
        Previous->NextSibling = Element->NextSibling;
      else
        Parent->FirstChild = Element->NextSibling;

      if (Parent->LastChild == Element)
        Parent->LastChild = Previous;

      break;
    }

    Previous = Current;
    Current = Current->NextSibling;
  }

  Element->Parent = NULL;
  Element->NextSibling = NULL;
}

int MyaAppendChild(MyaElement *Parent, MyaElement *Child) {
  if (!Parent || !Child || Parent == Child)
    return 0;

  for (MyaElement *Current = Parent; Current; Current = Current->Parent) {
    if (Current == Child) {
      return 0;
    }
  }

  MyaDetach(Child);

  Child->Parent = Parent;
  Child->NextSibling = NULL;
  if (Parent->LastChild) {
    Parent->LastChild->NextSibling = Child;
  } else {
    Parent->FirstChild = Child;
  }
  Parent->LastChild = Child;

  return 1;
}

void MyaSetAttribute(MyaElement *Element, const char *Key, const char *Value) {
  MyaSetEntry(Element->Attributes, Key, Value);
}

void MyaSetText(MyaElement *Element, const char *Text) {
  if (!Element) {
    return;
  }

  char *Copy = NULL;
  if (Text) {
    Copy = MyaStrdup(Text);

    if (!Copy) {
      return;
    }
  }

  free(Element->Text);
  Element->Text = Copy;
}

void MyaDestroy(MyaElement *Element) {
  if (!Element) {
    return;
  }

  MyaDetach(Element);

  MyaElement *Child = Element->FirstChild;
  while (Child) {
    MyaElement *Next = Child->NextSibling;
    Child->Parent = NULL;
    Child->NextSibling = NULL;
    MyaDestroy(Child);
    Child = Next;
  }

  free(Element->TagName);
  free(Element->Text);
  free(Element);
}
