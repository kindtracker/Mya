#include "Mya.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *MyaStyleStrndup(const char *String, size_t Length) {
  char *Copy = malloc(Length + 1);
  if (!Copy)
    return NULL;

  memcpy(Copy, String, Length);
  Copy[Length] = '\0';
  return Copy;
}

static char *MyaStyleTrim(const char *Start, size_t Length) {
  while (Length > 0 && isspace((unsigned char)*Start)) {
    Start++;
    Length--;
  }

  while (Length > 0 && isspace((unsigned char)Start[Length - 1]))
    Length--;

  return MyaStyleStrndup(Start, Length);
}

static bool MyaStyleEqualsIgnoreCase(const char *A, const char *B) {
  while (*A && *B) {
    if (tolower((unsigned char)*A) != tolower((unsigned char)*B))
      return false;

    A++;
    B++;
  }

  return *A == '\0' && *B == '\0';
}

static char *MyaStyleRemoveComments(const char *CSS) {
  size_t Length = strlen(CSS);
  char *Output = malloc(Length + 1);

  if (!Output)
    return NULL;

  size_t Read = 0;
  size_t Write = 0;
  char Quote = '\0';

  while (Read < Length) {
    char Character = CSS[Read];

    if (Quote) {
      Output[Write++] = Character;

      if (Character == '\\' && Read + 1 < Length) {
        Output[Write++] = CSS[++Read];
      } else if (Character == Quote) {
        Quote = '\0';
      }

      Read++;
      continue;
    }

    if (Character == '"' || Character == '\'') {
      Quote = Character;
      Output[Write++] = Character;
      Read++;
      continue;
    }

    if (Character == '/' && Read + 1 < Length && CSS[Read + 1] == '*') {
      Read += 2;

      while (Read + 1 < Length && !(CSS[Read] == '*' && CSS[Read + 1] == '/')) {
        Read++;
      }

      if (Read + 1 < Length)
        Read += 2;

      Output[Write++] = ' ';
      continue;
    }

    Output[Write++] = Character;
    Read++;
  }

  Output[Write] = '\0';
  return Output;
}

static const char *MyaStyleFindDelimiter(const char *Start, const char *End,
                                         char Delimiter) {
  char Quote = '\0';
  int Parentheses = 0;
  int Brackets = 0;

  for (const char *Current = Start; Current < End; Current++) {
    char Character = *Current;

    if (Quote) {
      if (Character == '\\' && Current + 1 < End) {
        Current++;
      } else if (Character == Quote) {
        Quote = '\0';
      }

      continue;
    }

    if (Character == '"' || Character == '\'') {
      Quote = Character;
      continue;
    }

    if (Character == '(') {
      Parentheses++;
      continue;
    }

    if (Character == ')' && Parentheses > 0) {
      Parentheses--;
      continue;
    }

    if (Character == '[') {
      Brackets++;
      continue;
    }

    if (Character == ']' && Brackets > 0) {
      Brackets--;
      continue;
    }

    if (Character == Delimiter && Parentheses == 0 && Brackets == 0)
      return Current;
  }

  return End;
}

static const char *MyaStyleFindBlockEnd(const char *Start, const char *End) {
  char Quote = '\0';
  int Parentheses = 0;
  int Brackets = 0;
  int Braces = 1;

  for (const char *Current = Start; Current < End; Current++) {
    char Character = *Current;

    if (Quote) {
      if (Character == '\\' && Current + 1 < End) {
        Current++;
      } else if (Character == Quote) {
        Quote = '\0';
      }

      continue;
    }

    if (Character == '"' || Character == '\'') {
      Quote = Character;
      continue;
    }

    if (Character == '(') {
      Parentheses++;
      continue;
    }

    if (Character == ')' && Parentheses > 0) {
      Parentheses--;
      continue;
    }

    if (Character == '[') {
      Brackets++;
      continue;
    }

    if (Character == ']' && Brackets > 0) {
      Brackets--;
      continue;
    }

    if (Parentheses != 0 || Brackets != 0)
      continue;

    if (Character == '{') {
      Braces++;
    } else if (Character == '}') {
      Braces--;

      if (Braces == 0)
        return Current;
    }
  }

  return End;
}

static bool MyaStyleEndsWithImportant(const char *Value) {
  size_t Length = strlen(Value);

  while (Length > 0 && isspace((unsigned char)Value[Length - 1]))
    Length--;

  if (Length < 10)
    return false;

  const char *Suffix = Value + Length - 10;

  if (Suffix[0] != '!')
    return false;

  return MyaStyleEqualsIgnoreCase(Suffix + 1, "important");
}

static void MyaStyleRemoveImportant(char *Value) {
  size_t Length = strlen(Value);

  while (Length > 0 && isspace((unsigned char)Value[Length - 1]))
    Length--;

  if (Length >= 10 && Value[Length - 10] == '!' &&
      MyaStyleEqualsIgnoreCase(Value + Length - 9, "important")) {
    Value[Length - 10] = '\0';
  }
}

static bool MyaStyleAddDeclaration(MyaStyleRule *Rule, const char *Start,
                                   size_t Length) {
  const char *End = Start + Length;
  const char *Colon = MyaStyleFindDelimiter(Start, End, ':');

  if (Colon == End)
    return true;

  char *Property = MyaStyleTrim(Start, (size_t)(Colon - Start));
  char *Value = MyaStyleTrim(Colon + 1, (size_t)(End - Colon - 1));

  if (!Property || !Value) {
    free(Property);
    free(Value);
    return false;
  }

  if (Property[0] == '\0' || Value[0] == '\0') {
    free(Property);
    free(Value);
    return true;
  }

  bool Important = MyaStyleEndsWithImportant(Value);

  if (Important)
    MyaStyleRemoveImportant(Value);

  MyaStyleDeclaration *Declaration = calloc(1, sizeof(*Declaration));

  if (!Declaration) {
    free(Property);
    free(Value);
    return false;
  }

  Declaration->Property = Property;
  Declaration->Value = Value;
  Declaration->Important = Important;

  MyaStyleDeclaration **Tail = &Rule->Declarations;

  while (*Tail)
    Tail = &(*Tail)->Next;

  *Tail = Declaration;
  return true;
}

static bool MyaStyleParseDeclarations(MyaStyleRule *Rule, const char *Start,
                                      const char *End) {
  const char *Current = Start;

  while (Current < End) {
    const char *Semicolon = MyaStyleFindDelimiter(Current, End, ';');

    if (!MyaStyleAddDeclaration(Rule, Current, (size_t)(Semicolon - Current))) {
      return false;
    }

    if (Semicolon == End)
      break;

    Current = Semicolon + 1;
  }

  return true;
}

static void MyaStyleDestroyRule(MyaStyleRule *Rule) {
  if (!Rule)
    return;

  free(Rule->Selector);

  MyaStyleDeclaration *Declaration = Rule->Declarations;

  while (Declaration) {
    MyaStyleDeclaration *Next = Declaration->Next;

    free(Declaration->Property);
    free(Declaration->Value);
    free(Declaration);

    Declaration = Next;
  }

  free(Rule);
}

void MyaStyleSheetDestroy(MyaStyleSheet *Sheet) {
  if (!Sheet)
    return;

  MyaStyleRule *Rule = Sheet->Rules;

  while (Rule) {
    MyaStyleRule *Next = Rule->Next;
    MyaStyleDestroyRule(Rule);
    Rule = Next;
  }

  free(Sheet);
}

MyaStyleSheet *MyaStyleParse(const char *CSS) {
  if (!CSS)
    return NULL;

  char *CleanCSS = MyaStyleRemoveComments(CSS);

  if (!CleanCSS)
    return NULL;

  MyaStyleSheet *Sheet = calloc(1, sizeof(*Sheet));

  if (!Sheet) {
    free(CleanCSS);
    return NULL;
  }

  const char *Current = CleanCSS;
  const char *End = CleanCSS + strlen(CleanCSS);
  MyaStyleRule **RuleTail = &Sheet->Rules;

  while (Current < End) {
    while (Current < End && isspace((unsigned char)*Current))
      Current++;

    if (Current >= End)
      break;

    // TODO: implement @media and @import.
    if (*Current == '@') {
      const char *Semicolon = MyaStyleFindDelimiter(Current, End, ';');

      const char *OpenBrace = MyaStyleFindDelimiter(Current, End, '{');

      if (OpenBrace < End && OpenBrace < Semicolon) {
        const char *BlockEnd = MyaStyleFindBlockEnd(OpenBrace + 1, End);

        Current = BlockEnd < End ? BlockEnd + 1 : End;
      } else {
        Current = Semicolon < End ? Semicolon + 1 : End;
      }

      continue;
    }

    const char *OpenBrace = MyaStyleFindDelimiter(Current, End, '{');

    if (OpenBrace == End)
      break;

    char *Selector = MyaStyleTrim(Current, (size_t)(OpenBrace - Current));

    if (!Selector) {
      MyaStyleSheetDestroy(Sheet);
      free(CleanCSS);
      return NULL;
    }

    const char *CloseBrace = MyaStyleFindBlockEnd(OpenBrace + 1, End);

    if (CloseBrace == End) {
      free(Selector);
      break;
    }

    if (Selector[0] != '\0') {
      MyaStyleRule *Rule = calloc(1, sizeof(*Rule));

      if (!Rule) {
        free(Selector);
        MyaStyleSheetDestroy(Sheet);
        free(CleanCSS);
        return NULL;
      }

      Rule->Selector = Selector;

      if (!MyaStyleParseDeclarations(Rule, OpenBrace + 1, CloseBrace)) {
        MyaStyleDestroyRule(Rule);
        MyaStyleSheetDestroy(Sheet);
        free(CleanCSS);
        return NULL;
      }

      *RuleTail = Rule;
      RuleTail = &Rule->Next;
    } else {
      free(Selector);
    }

    Current = CloseBrace + 1;
  }

  free(CleanCSS);
  return Sheet;
}
