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

static MyaEntry *MyaSetEntry(MyaEntry *Entries, const char *Name,
                             const char *Value) {
  MyaEntry *Current = Entries;

  while (Current) {
    if (strcmp(Name, Current->Name) == 0) {
      char *Copy = MyaStrdup(Value);
      if (!Copy)
        return Entries;

      free(Current->Value);
      Current->Value = Copy;
      return Entries;
    }

    if (!Current->Next)
      break;

    Current = Current->Next;
  }

  MyaEntry *NewEntry = malloc(sizeof(MyaEntry));
  if (!NewEntry)
    return Entries;

  NewEntry->Name = MyaStrdup(Name);
  NewEntry->Value = MyaStrdup(Value);

  if (!NewEntry->Name || !NewEntry->Value) {
    free(NewEntry->Name);
    free(NewEntry->Value);
    free(NewEntry);
    return Entries;
  }

  NewEntry->Next = NULL;

  if (Current)
    Current->Next = NewEntry;
  else
    Entries = NewEntry;

  return Entries;
}

const char *MyaGetStyle(MyaElement *Element, const char *Key) {
  if (!Element || !Key)
    return NULL;

  MyaEntry *Current = Element->Styles;

  while (Current) {
    if (strcmp(Current->Name, Key) == 0) {
      return Current->Value;
    }

    Current = Current->Next;
  }

  return NULL;
}

#define MYA_STYLE(Name, Value)                                                 \
  Element->Styles = MyaSetEntry(Element->Styles, #Name, Value)
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
  MYA_STYLE(LetterSpacing, "normal");
  MYA_STYLE(WordSpacing, "normal");
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

  if (strcmp(TagName, "h1") == 0) {
    MYA_STYLE(FontSize, "32");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "21.44");
    MYA_STYLE(MarginBottom, "21.44");
  } else if (strcmp(TagName, "h2") == 0) {
    MYA_STYLE(FontSize, "24");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "19.92");
    MYA_STYLE(MarginBottom, "19.92");
  } else if (strcmp(TagName, "h3") == 0) {
    MYA_STYLE(FontSize, "18.72");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "18.72");
    MYA_STYLE(MarginBottom, "18.72");
  } else if (strcmp(TagName, "h4") == 0) {
    MYA_STYLE(FontSize, "16");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "21.28");
    MYA_STYLE(MarginBottom, "21.28");
  } else if (strcmp(TagName, "h5") == 0) {
    MYA_STYLE(FontSize, "13.28");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "22.18");
    MYA_STYLE(MarginBottom, "22.18");
  } else if (strcmp(TagName, "h6") == 0) {
    MYA_STYLE(FontSize, "10.72");
    MYA_STYLE(FontWeight, "700");
    MYA_STYLE(MarginTop, "24.14");
    MYA_STYLE(MarginBottom, "24.14");
  } else if (strcmp(TagName, "p") == 0) {
    MYA_STYLE(MarginTop, "16");
    MYA_STYLE(MarginBottom, "16");
  } else if (strcmp(TagName, "small") == 0) {
    MYA_STYLE(FontSize, "13.33");
  } else if (strcmp(TagName, "big") == 0) {
    MYA_STYLE(FontSize, "19.2");
  } else if (strcmp(TagName, "strong") == 0 || strcmp(TagName, "b") == 0) {
    MYA_STYLE(FontWeight, "700");
  } else if (strcmp(TagName, "em") == 0 || strcmp(TagName, "i") == 0 ||
             strcmp(TagName, "cite") == 0 || strcmp(TagName, "dfn") == 0 ||
             strcmp(TagName, "var") == 0) {
    MYA_STYLE(FontStyle, "italic");
  } else if (strcmp(TagName, "u") == 0 || strcmp(TagName, "ins") == 0) {
    MYA_STYLE(TextDecoration, "underline");
  } else if (strcmp(TagName, "s") == 0 || strcmp(TagName, "del") == 0 ||
             strcmp(TagName, "strike") == 0) {
    MYA_STYLE(TextDecoration, "line-through");
  } else if (strcmp(TagName, "mark") == 0) {
    MYA_STYLE(BackgroundColor, "#ffff00");
  } else if (strcmp(TagName, "sub") == 0) {
    MYA_STYLE(FontSize, "13.33");
  } else if (strcmp(TagName, "sup") == 0) {
    MYA_STYLE(FontSize, "13.33");
  } else if (strcmp(TagName, "pre") == 0) {
    MYA_STYLE(FontFamily, "monospace");
    MYA_STYLE(WhiteSpace, "pre");
    MYA_STYLE(MarginTop, "13.33");
    MYA_STYLE(MarginBottom, "13.33");
  } else if (strcmp(TagName, "code") == 0 || strcmp(TagName, "kbd") == 0 ||
             strcmp(TagName, "samp") == 0) {
    MYA_STYLE(FontFamily, "monospace");
    MYA_STYLE(FontSize, "13.33");
  } else if (strcmp(TagName, "blockquote") == 0) {
    MYA_STYLE(MarginLeft, "40");
    MYA_STYLE(MarginRight, "40");
    MYA_STYLE(MarginTop, "16");
    MYA_STYLE(MarginBottom, "16");
  } else if (strcmp(TagName, "ul") == 0 || strcmp(TagName, "ol") == 0) {
    MYA_STYLE(PaddingLeft, "40");
    MYA_STYLE(MarginTop, "16");
    MYA_STYLE(MarginBottom, "16");
  } else if (strcmp(TagName, "li") == 0) {
    MYA_STYLE(Display, "list-item");
  } else if (strcmp(TagName, "hr") == 0) {
    MYA_STYLE(BorderWidth, "1");
    MYA_STYLE(BorderStyle, "inset");
    MYA_STYLE(MarginTop, "8");
    MYA_STYLE(MarginBottom, "8");
  } else if (strcmp(TagName, "img") == 0) {
    MYA_STYLE(Display, "inline");
  } else if (strcmp(TagName, "span") == 0 || strcmp(TagName, "a") == 0 ||
             strcmp(TagName, "strong") == 0 || strcmp(TagName, "b") == 0 ||
             strcmp(TagName, "em") == 0 || strcmp(TagName, "i") == 0 ||
             strcmp(TagName, "small") == 0 || strcmp(TagName, "code") == 0 ||
             strcmp(TagName, "label") == 0) {
    MYA_STYLE(Display, "inline");
  }

  if (strcmp(TagName, "a") == 0) {
    MYA_STYLE(Color, "#0000ee");
    MYA_STYLE(TextDecoration, "underline");
    MYA_STYLE(Cursor, "pointer");
  }

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
  if (!Element)
    return;

  Element->Attributes = MyaSetEntry(Element->Attributes, Key, Value);
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
