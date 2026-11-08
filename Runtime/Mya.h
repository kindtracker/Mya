#pragma once
#include "Clay.h"
#include <SDL3/SDL.h>
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
int MyaAppendChild(MyaElement *Parent, MyaElement *Child);
void MyaSetAttribute(MyaElement *Element, const char *Key, const char *Value);
void MyaSetText(MyaElement *Element, const char *Text);
void MyaDestroy(MyaElement *Element);

MyaElement *MyaCreatePage();

// Internal
extern SDL_Window *Window;
extern SDL_Renderer *Renderer;

void MyaGenerateClayLayout(MyaElement *Element);
void MyaRenderClay(Clay_RenderCommandArray Commands);
