#include "Mya.h"

#include "Clay.h"

void MyaGenerateClayLayout(MyaElement *Element) {
  if (!Element) {
    return;
  }

  CLAY_AUTO_ID(
      {.layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                             .height = CLAY_SIZING_FIT(0)},
                  .padding = {.left = 8, .right = 8, .top = 8, .bottom = 8},
                  .childGap = 4,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM},
       .backgroundColor = {40, 40, 48, 255}}) {
    if (Element->Text && Element->Text[0] != '\0') {
      Clay_String Text = {.isStaticallyAllocated = false,
                          .length = (int32_t)strlen(Element->Text),
                          .chars = Element->Text};

      CLAY_TEXT(Text, {.fontSize = 8, .textColor = {240, 240, 240, 255}});
    }

    for (MyaElement *Child = Element->FirstChild; Child;
         Child = Child->NextSibling) {
      MyaGenerateClayLayout(Child);
    }
  }
}
