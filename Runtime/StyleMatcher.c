#include <ctype.h>
#include <stdio.h>

#include "Mya.h"

static void MyaStylePropertyToName(const char *Property, char *Name) {
  if (!Property || !Name) {
    return;
  }

  bool Uppercase = true;

  while (*Property) {
    if (*Property == '-') {
      Uppercase = true;
    } else {
      *Name++ = Uppercase ? (char)toupper((unsigned char)*Property) : *Property;
      Uppercase = false;
    }

    Property++;
  }

  *Name = '\0';
}

static void MyaAppendStyleSheet(MyaStyleSheet *Destination,
                                MyaStyleSheet *Source) {
  if (!Source) {
    return;
  }

  if (!Source->Rules) {
    MyaStyleSheetDestroy(Source);
    return;
  }

  MyaStyleRule **Tail = &Destination->Rules;

  while (*Tail) {
    Tail = &(*Tail)->Next;
  }

  *Tail = Source->Rules;
  Source->Rules = NULL;

  MyaStyleSheetDestroy(Source);
}

static bool MyaCollectStyles(MyaElement *Element, MyaStyleSheet *StyleSheet) {
  if (!Element) {
    return true;
  }

  if (strcmp(Element->TagName, "style") == 0) {
    if (Element->Text) {
      MyaStyleSheet *Parsed = MyaStyleParse(Element->Text);

      if (!Parsed) {
        return false;
      }

      MyaAppendStyleSheet(StyleSheet, Parsed);
    }
  }

  for (MyaElement *Child = Element->FirstChild; Child;
       Child = Child->NextSibling) {
    if (!MyaCollectStyles(Child, StyleSheet)) {
      return false;
    }
  }

  return true;
}

MyaStyleSheet *MyaGetStyleSheet(MyaElement *Page) {
  if (!Page) {
    return NULL;
  }

  MyaStyleSheet *StyleSheet = calloc(1, sizeof(MyaStyleSheet));
  if (!StyleSheet) {
    return NULL;
  }
  if (!MyaCollectStyles(Page, StyleSheet)) {
    MyaStyleSheetDestroy(StyleSheet);
    return NULL;
  }

  return StyleSheet;
}

static void MyaApplyStylesToElement(MyaElement *Element,
                                    MyaStyleSheet *StyleSheet) {
  if (!Element) {
    return;
  }

  for (MyaStyleRule *Rule = StyleSheet->Rules; Rule; Rule = Rule->Next) {
    if (strcmp(Rule->Selector, Element->TagName) != 0) {
      continue;
    }

    for (MyaStyleDeclaration *Declaration = Rule->Declarations; Declaration;
         Declaration = Declaration->Next) {
      size_t Length = strlen(Declaration->Property);
      char StyleName[Length + 1];
      MyaStylePropertyToName(Declaration->Property, StyleName);
      MyaSetStyle(Element, StyleName, Declaration->Value);
    }
  }

  for (MyaElement *Child = Element->FirstChild; Child;
       Child = Child->NextSibling) {
    MyaApplyStylesToElement(Child, StyleSheet);
  }
}

bool MyaApplyStyles(MyaElement *Page) {
  if (!Page) {
    return false;
  }

  MyaStyleSheet *StyleSheet = MyaGetStyleSheet(Page);

  if (!StyleSheet) {
    return false;
  }

  MyaApplyStylesToElement(Page, StyleSheet);

  MyaStyleSheetDestroy(StyleSheet);
  return true;
}
