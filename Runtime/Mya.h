#pragma once
#include <stddef.h>

typedef struct MyaElement {
  char *TagName;
  char *Text;
  struct MyaElement *Parent;
  struct MyaElement *FirstChild;
  struct MyaElement *LastChild;
  struct MyaElement *NextSibling;
} MyaElement;

MyaElement *MyaCreate(const char *TagName);
int MyaAppendChild(MyaElement *Parent, MyaElement *Child);
void MyaSetText(MyaElement *Element, const char *Text);
void MyaDestroy(MyaElement *Element);

MyaElement *MyaCreatePage();
