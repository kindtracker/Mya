#include "Mya.h"

#include "Clay.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <wchar.h>

static int MyaHexValue(char Character) {
  if (Character >= '0' && Character <= '9')
    return Character - '0';
  if (Character >= 'a' && Character <= 'f')
    return Character - 'a' + 10;
  if (Character >= 'A' && Character <= 'F')
    return Character - 'A' + 10;

  return -1;
}

static Clay_Color MyaParseColor(const char *Value) {
  Clay_Color Color = {0, 0, 0, 255};

  if (!Value)
    return Color;

  if (strcmp(Value, "transparent") == 0) {
    Color.a = 0;
    return Color;
  }

  if (Value[0] != '#')
    return Color;

  size_t Length = strlen(Value + 1);

  if (Length == 3 || Length == 4) {
    int R = MyaHexValue(Value[1]);
    int G = MyaHexValue(Value[2]);
    int B = MyaHexValue(Value[3]);
    int A = Length == 4 ? MyaHexValue(Value[4]) : 15;

    if (R < 0 || G < 0 || B < 0 || A < 0)
      return Color;

    Color.r = (uint8_t)(R * 17);
    Color.g = (uint8_t)(G * 17);
    Color.b = (uint8_t)(B * 17);
    Color.a = (uint8_t)(A * 17);
  } else if (Length == 6 || Length == 8) {
    int Values[8];

    for (size_t I = 0; I < Length; I++) {
      Values[I] = MyaHexValue(Value[I + 1]);
      if (Values[I] < 0)
        return Color;
    }

    Color.r = (uint8_t)(Values[0] * 16 + Values[1]);
    Color.g = (uint8_t)(Values[2] * 16 + Values[3]);
    Color.b = (uint8_t)(Values[4] * 16 + Values[5]);

    if (Length == 8)
      Color.a = (uint8_t)(Values[6] * 16 + Values[7]);
  }

  return Color;
}

static bool MyaUnitEquals(const char *A, const char *B) {
  while (*A && *B) {
    if (tolower((unsigned char)*A) != tolower((unsigned char)*B)) {
      return false;
    }

    A++;
    B++;
  }

  return *A == *B;
}

static MyaUnit MyaParseUnit(const char *Value) {
  MyaUnit Result = {.Value = 0, .Type = MYA_UNIT_NONE, .Valid = false};

  if (!Value)
    return Result;

  char *End;
  float Number = strtof(Value, &End);

  if (End == Value || !isfinite(Number))
    return Result;

  while (isspace((unsigned char)*End))
    End++;

  Result.Value = Number;
  Result.Valid = true;

  if (*End == '\0') {
    Result.Type = MYA_UNIT_NONE;
  } else if (MyaUnitEquals(End, "px")) {
    Result.Type = MYA_UNIT_PX;
  } else if (MyaUnitEquals(End, "em")) {
    Result.Type = MYA_UNIT_EM;
  } else if (MyaUnitEquals(End, "rem")) {
    Result.Type = MYA_UNIT_REM;
  } else if (MyaUnitEquals(End, "%")) {
    Result.Type = MYA_UNIT_PERCENT;
  } else if (MyaUnitEquals(End, "vw")) {
    Result.Type = MYA_UNIT_VW;
  } else if (MyaUnitEquals(End, "vh")) {
    Result.Type = MYA_UNIT_VH;
  } else if (MyaUnitEquals(End, "vmin")) {
    Result.Type = MYA_UNIT_VMIN;
  } else if (MyaUnitEquals(End, "vmax")) {
    Result.Type = MYA_UNIT_VMAX;
  } else if (MyaUnitEquals(End, "pt")) {
    Result.Type = MYA_UNIT_PT;
  } else if (MyaUnitEquals(End, "pc")) {
    Result.Type = MYA_UNIT_PC;
  } else if (MyaUnitEquals(End, "in")) {
    Result.Type = MYA_UNIT_IN;
  } else if (MyaUnitEquals(End, "cm")) {
    Result.Type = MYA_UNIT_CM;
  } else if (MyaUnitEquals(End, "mm")) {
    Result.Type = MYA_UNIT_MM;
  } else {
    Result.Valid = false;
  }

  return Result;
}

static float MyaUnitToPixels(MyaUnit Unit, MyaUnitContext Context,
                             MyaReferenceType Reference) {
  switch (Unit.Type) {
  case MYA_UNIT_NONE:
  case MYA_UNIT_PX:
    return Unit.Value;

  case MYA_UNIT_EM:
    return Unit.Value * Context.FontSize;

  case MYA_UNIT_REM:
    return Unit.Value * Context.RootFontSize;

  case MYA_UNIT_PERCENT:
    return Unit.Value *
           (Reference == MYA_REFERENCE_WIDTH ? Context.ReferenceWidth
                                             : Context.ReferenceHeight) /
           100.0f;

  case MYA_UNIT_VW:
    return Unit.Value * Context.ViewportWidth / 100.0f;

  case MYA_UNIT_VH:
    return Unit.Value * Context.ViewportHeight / 100.0f;

  case MYA_UNIT_VMIN:
    return Unit.Value * fminf(Context.ViewportWidth, Context.ViewportHeight) /
           100.0f;

  case MYA_UNIT_VMAX:
    return Unit.Value * fmaxf(Context.ViewportWidth, Context.ViewportHeight) /
           100.0f;

  case MYA_UNIT_PT:
    return Unit.Value * (96.0f / 72.0f);

  case MYA_UNIT_PC:
    return Unit.Value * 16.0f;

  case MYA_UNIT_IN:
    return Unit.Value * 96.0f;

  case MYA_UNIT_CM:
    return Unit.Value * (96.0f / 2.54f);

  case MYA_UNIT_MM:
    return Unit.Value * (96.0f / 25.4f);

  default:
    return 0;
  }
}

int MyaFindFont(const char *FontFamily, int FontSize) {
  for (int Idx = 0; Idx < FontCount; Idx++) {
    if (strcmp(Fonts[Idx].FontName, FontFamily) == 0 &&
        Fonts[Idx].FontSize == FontSize) {
      return Idx;
    }
  }

  MyaFontEntry *NewFonts =
      realloc(Fonts, sizeof(MyaFontEntry) * (FontCount + 1));
  if (!NewFonts) {
    return -1;
  }
  Fonts = NewFonts;
  Fonts[FontCount] = (MyaFontEntry){
      .FontName = FontFamily, .FontSize = FontSize, .Font = NULL, .Next = NULL};
  return FontCount++;
}

void MyaGenerateClayLayout(MyaElement *Element, MyaUnitContext *ParentContext) {
  if (!Element) {
    return;
  }

  MyaUnitContext Context = {0};
  Context = *ParentContext;

  const char *FontFamily = MyaGetStyle(Element, "FontFamily");

  MyaUnit FontSizeUnit = MyaParseUnit(MyaGetStyle(Element, "FontSize"));
  float FontSize = FontSizeUnit.Valid ? MyaUnitToPixels(FontSizeUnit, Context,
                                                        MYA_REFERENCE_WIDTH)
                                      : 16.0f;

  float FontWeight = atof(MyaGetStyle(Element, "FontWeight"));
  const char *FontStyle = MyaGetStyle(Element, "FontStyle");

  MyaUnit LineHeightUnit = MyaParseUnit(MyaGetStyle(Element, "LineHeight"));
  float LineHeight =
      LineHeightUnit.Valid
          ? MyaUnitToPixels(LineHeightUnit, Context, MYA_REFERENCE_HEIGHT)
          : 0;

  MyaUnit LetterSpacingUnit =
      MyaParseUnit(MyaGetStyle(Element, "LetterSpacing"));
  float LetterSpacing =
      LetterSpacingUnit.Valid
          ? MyaUnitToPixels(LetterSpacingUnit, Context, MYA_REFERENCE_WIDTH)
          : 0;

  MyaUnit WordSpacingUnit = MyaParseUnit(MyaGetStyle(Element, "WordSpacing"));
  float WordSpacing =
      WordSpacingUnit.Valid
          ? MyaUnitToPixels(WordSpacingUnit, Context, MYA_REFERENCE_WIDTH)
          : 0;

  const char *TextAlign = MyaGetStyle(Element, "TextAlign");
  const char *TextDecoration = MyaGetStyle(Element, "TextDecoration");
  const char *TextTransform = MyaGetStyle(Element, "TextTransform");

  MyaUnit TextIndentUnit = MyaParseUnit(MyaGetStyle(Element, "TextIndent"));
  float TextIndent =
      TextIndentUnit.Valid
          ? MyaUnitToPixels(TextIndentUnit, Context, MYA_REFERENCE_WIDTH)
          : 0;

  const char *WhiteSpace = MyaGetStyle(Element, "WhiteSpace");
  const char *WordBreak = MyaGetStyle(Element, "WordBreak");
  const char *OverflowWrap = MyaGetStyle(Element, "OverflowWrap");

  Clay_Color Color = MyaParseColor(MyaGetStyle(Element, "Color"));
  Clay_Color BackgroundColor =
      MyaParseColor(MyaGetStyle(Element, "BackgroundColor"));

  const char *Display = MyaGetStyle(Element, "Display");
  const char *Position = MyaGetStyle(Element, "Position");

  const char *Width = MyaGetStyle(Element, "Width");
  const char *Height = MyaGetStyle(Element, "Height");
  const char *MinWidth = MyaGetStyle(Element, "MinWidth");
  const char *MinHeight = MyaGetStyle(Element, "MinHeight");
  const char *MaxWidth = MyaGetStyle(Element, "MaxWidth");
  const char *MaxHeight = MyaGetStyle(Element, "MaxHeight");

#define MYA_GET_UNIT(Property, Reference)                                      \
  ({                                                                           \
    MyaUnit Unit = MyaParseUnit(MyaGetStyle(Element, Property));               \
    Unit.Valid ? MyaUnitToPixels(Unit, Context, Reference) : 0.0f;             \
  })

  float MarginTop = MYA_GET_UNIT("MarginTop", MYA_REFERENCE_WIDTH);
  float MarginRight = MYA_GET_UNIT("MarginRight", MYA_REFERENCE_WIDTH);
  float MarginBottom = MYA_GET_UNIT("MarginBottom", MYA_REFERENCE_WIDTH);
  float MarginLeft = MYA_GET_UNIT("MarginLeft", MYA_REFERENCE_WIDTH);

  float PaddingTop = MYA_GET_UNIT("PaddingTop", MYA_REFERENCE_WIDTH);
  float PaddingRight = MYA_GET_UNIT("PaddingRight", MYA_REFERENCE_WIDTH);
  float PaddingBottom = MYA_GET_UNIT("PaddingBottom", MYA_REFERENCE_WIDTH);
  float PaddingLeft = MYA_GET_UNIT("PaddingLeft", MYA_REFERENCE_WIDTH);

  float BorderWidth = MYA_GET_UNIT("BorderWidth", MYA_REFERENCE_WIDTH);

  Clay_Color BorderColor = MyaParseColor(MyaGetStyle(Element, "BorderColor"));
  const char *BorderStyle = MyaGetStyle(Element, "BorderStyle");

  float BorderRadius = MYA_GET_UNIT("BorderRadius", MYA_REFERENCE_WIDTH);

  const char *FlexDirection = MyaGetStyle(Element, "FlexDirection");
  const char *FlexWrap = MyaGetStyle(Element, "FlexWrap");
  const char *JustifyContent = MyaGetStyle(Element, "JustifyContent");
  const char *AlignItems = MyaGetStyle(Element, "AlignItems");
  const char *AlignContent = MyaGetStyle(Element, "AlignContent");
  const char *AlignSelf = MyaGetStyle(Element, "AlignSelf");

  float FlexGrow = atof(MyaGetStyle(Element, "FlexGrow"));
  float FlexShrink = atof(MyaGetStyle(Element, "FlexShrink"));

  const char *FlexBasis = MyaGetStyle(Element, "FlexBasis");

  float Gap = MYA_GET_UNIT("Gap", MYA_REFERENCE_WIDTH);
  float RowGap = MYA_GET_UNIT("RowGap", MYA_REFERENCE_WIDTH);
  float ColumnGap = MYA_GET_UNIT("ColumnGap", MYA_REFERENCE_WIDTH);

  const char *Overflow = MyaGetStyle(Element, "Overflow");
  const char *OverflowX = MyaGetStyle(Element, "OverflowX");
  const char *OverflowY = MyaGetStyle(Element, "OverflowY");

  float Opacity = atof(MyaGetStyle(Element, "Opacity"));

  const char *Visibility = MyaGetStyle(Element, "Visibility");
  const char *ZIndex = MyaGetStyle(Element, "ZIndex");
  const char *Cursor = MyaGetStyle(Element, "Cursor");

#undef MYA_GET_UNIT

  Context.FontSize = FontSize;

  int FontId = MyaFindFont(FontFamily, FontSize);

  CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                                      .height = CLAY_SIZING_FIT(0)},
                           .padding = {.left = PaddingLeft,
                                       .right = PaddingRight,
                                       .top = PaddingTop,
                                       .bottom = PaddingBottom},
                           .childGap = 4,
                           .layoutDirection = CLAY_TOP_TO_BOTTOM},
                .backgroundColor = BackgroundColor}) {
    if (Element->Text && Element->Text[0] != '\0') {
      Clay_String Text = {.isStaticallyAllocated = false,
                          .length = (int32_t)strlen(Element->Text),
                          .chars = Element->Text};

      CLAY_TEXT(Text,
                {.fontSize = FontSize, .textColor = Color, .fontId = FontId});
    }

    for (MyaElement *Child = Element->FirstChild; Child;
         Child = Child->NextSibling) {
      MyaGenerateClayLayout(Child, &Context);
    }
  }
}
