#include "Mya.h"

#include "Clay.h"

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

void MyaGenerateClayLayout(MyaElement *Element) {
  if (!Element) {
    return;
  }

  const char *FontFamily = MyaGetStyle(Element, "FontFamily");
  float FontSize = atof(MyaGetStyle(Element, "FontSize"));
  float FontWeight = atof(MyaGetStyle(Element, "FontWeight"));
  const char *FontStyle = MyaGetStyle(Element, "FontStyle");
  const char *LineHeight = MyaGetStyle(Element, "LineHeight");
  float LetterSpacing = atof(MyaGetStyle(Element, "LetterSpacing"));
  float WordSpacing = atof(MyaGetStyle(Element, "WordSpacing"));
  const char *TextAlign = MyaGetStyle(Element, "TextAlign");
  const char *TextDecoration = MyaGetStyle(Element, "TextDecoration");
  const char *TextTransform = MyaGetStyle(Element, "TextTransform");
  float TextIndent = atof(MyaGetStyle(Element, "TextIndent"));
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
  float MarginTop = atof(MyaGetStyle(Element, "MarginTop"));
  float MarginRight = atof(MyaGetStyle(Element, "MarginRight"));
  float MarginBottom = atof(MyaGetStyle(Element, "MarginBottom"));
  float MarginLeft = atof(MyaGetStyle(Element, "MarginLeft"));
  float PaddingTop = atof(MyaGetStyle(Element, "PaddingTop"));
  float PaddingRight = atof(MyaGetStyle(Element, "PaddingRight"));
  float PaddingBottom = atof(MyaGetStyle(Element, "PaddingBottom"));
  float PaddingLeft = atof(MyaGetStyle(Element, "PaddingLeft"));
  float BorderWidth = atof(MyaGetStyle(Element, "BorderWidth"));
  Clay_Color BorderColor = MyaParseColor(MyaGetStyle(Element, "BorderColor"));
  const char *BorderStyle = MyaGetStyle(Element, "BorderStyle");
  float BorderRadius = atof(MyaGetStyle(Element, "BorderRadius"));
  const char *FlexDirection = MyaGetStyle(Element, "FlexDirection");
  const char *FlexWrap = MyaGetStyle(Element, "FlexWrap");
  const char *JustifyContent = MyaGetStyle(Element, "JustifyContent");
  const char *AlignItems = MyaGetStyle(Element, "AlignItems");
  const char *AlignContent = MyaGetStyle(Element, "AlignContent");
  const char *AlignSelf = MyaGetStyle(Element, "AlignSelf");
  float FlexGrow = atof(MyaGetStyle(Element, "FlexGrow"));
  float FlexShrink = atof(MyaGetStyle(Element, "FlexShrink"));
  const char *FlexBasis = MyaGetStyle(Element, "FlexBasis");
  float Gap = atof(MyaGetStyle(Element, "Gap"));
  float RowGap = atof(MyaGetStyle(Element, "RowGap"));
  float ColumnGap = atof(MyaGetStyle(Element, "ColumnGap"));
  const char *Overflow = MyaGetStyle(Element, "Overflow");
  const char *OverflowX = MyaGetStyle(Element, "OverflowX");
  const char *OverflowY = MyaGetStyle(Element, "OverflowY");
  float Opacity = atof(MyaGetStyle(Element, "Opacity"));
  const char *Visibility = MyaGetStyle(Element, "Visibility");
  const char *ZIndex = MyaGetStyle(Element, "ZIndex");
  const char *Cursor = MyaGetStyle(Element, "Cursor");

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

      CLAY_TEXT(Text, {.fontSize = FontSize, .textColor = Color});
    }

    for (MyaElement *Child = Element->FirstChild; Child;
         Child = Child->NextSibling) {
      MyaGenerateClayLayout(Child);
    }
  }
}
