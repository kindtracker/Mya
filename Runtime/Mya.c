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

  return Element;
}

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
