#pragma once
#include "Clay.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stddef.h>

typedef struct MyaEntry {
  char *Name;
  char *Value;
  struct MyaEntry *Next;
} MyaEntry;

typedef struct MyaElement {
  char *TagName;
  char *Text;
  MyaEntry *Attributes;
  MyaEntry *Styles;
  struct MyaElement *Parent;
  struct MyaElement *FirstChild;
  struct MyaElement *LastChild;
  struct MyaElement *NextSibling;
} MyaElement;

MyaElement *MyaCreate(const char *TagName);
void MyaDestroy(MyaElement *Element);
int MyaAppendChild(MyaElement *Parent, MyaElement *Child);
void MyaSetAttribute(MyaElement *Element, const char *Key, const char *Value);
void MyaSetStyle(MyaElement *Element, const char *Key, const char *Value);
void MyaSetText(MyaElement *Element, const char *Text);
const char *MyaGetStyle(MyaElement *Element, const char *Key);

MyaElement *MyaCreatePage();

typedef enum {
  MYA_UNIT_NONE,
  MYA_UNIT_PX,
  MYA_UNIT_EM,
  MYA_UNIT_REM,
  MYA_UNIT_PERCENT,
  MYA_UNIT_VW,
  MYA_UNIT_VH,
  MYA_UNIT_VMIN,
  MYA_UNIT_VMAX,
  MYA_UNIT_PT,
  MYA_UNIT_PC,
  MYA_UNIT_IN,
  MYA_UNIT_CM,
  MYA_UNIT_MM
} MyaUnitType;

typedef enum { MYA_REFERENCE_WIDTH, MYA_REFERENCE_HEIGHT } MyaReferenceType;

typedef struct {
  float Value;
  MyaUnitType Type;
  bool Valid;
} MyaUnit;

typedef struct {
  float FontSize;
  float RootFontSize;
  float ViewportWidth;
  float ViewportHeight;
  float ReferenceWidth;
  float ReferenceHeight;
} MyaUnitContext;

typedef struct MyaStyleDeclaration {
  char *Property;
  char *Value;
  bool Important;
  struct MyaStyleDeclaration *Next;
} MyaStyleDeclaration;

typedef struct MyaStyleRule {
  char *Selector;
  MyaStyleDeclaration *Declarations;
  struct MyaStyleRule *Next;
} MyaStyleRule;

typedef struct MyaStyleSheet {
  MyaStyleRule *Rules;
} MyaStyleSheet;

typedef struct MyaFontEntry {
  const char *FontName;
  int FontSize;
  TTF_Font *Font;
  struct MyaFontEntry *Next;
} MyaFontEntry;

extern SDL_Window *Window;
extern SDL_Renderer *Renderer;
extern MyaFontEntry *Fonts;
extern int FontCount;

void MyaGenerateClayLayout(MyaElement *Element, MyaUnitContext *ParentContext);
void MyaRenderClay(Clay_RenderCommandArray Commands);
void MyaUpdateFonts();
MyaStyleSheet *MyaStyleParse(const char *CSS);
void MyaStyleSheetDestroy(MyaStyleSheet *Sheet);
bool MyaApplyStyles(MyaElement *Page);
