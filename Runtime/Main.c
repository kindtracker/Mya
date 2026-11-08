#include "Mya.h"

#include <SDL3/SDL.h>

#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAY_IMPLEMENTATION
#include "Clay.h"

extern MyaElement *MyaCreatePage(void);

static SDL_Window *Window;
static SDL_Renderer *Renderer;

static Clay_Dimensions MeasureText(Clay_StringSlice Text,
                                   Clay_TextElementConfig *Config,
                                   void *UserData) {
  (void)Config;
  (void)UserData;

  return (Clay_Dimensions){.width = (float)Text.length * 8.0f, .height = 8.0f};
}

static void HandleClayError(Clay_ErrorData Error) {
  fprintf(stderr, "Clay error: %.*s\n", Error.errorText.length,
          Error.errorText.chars);
}

static void MyaGenerateClayLayout(MyaElement *Element) {
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

static void RenderClay(Clay_RenderCommandArray Commands) {
  for (int32_t i = 0; i < Commands.length; i++) {
    Clay_RenderCommand *Command = &Commands.internalArray[i];
    Clay_BoundingBox Box = Command->boundingBox;
    SDL_FRect Rect = {.x = Box.x, .y = Box.y, .w = Box.width, .h = Box.height};

    switch (Command->commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
      Clay_Color Color = Command->renderData.rectangle.backgroundColor;
      SDL_SetRenderDrawColor(Renderer, (Uint8)Color.r, (Uint8)Color.g,
                             (Uint8)Color.b, (Uint8)Color.a);
      SDL_RenderFillRect(Renderer, &Rect);
      break;
    }

    case CLAY_RENDER_COMMAND_TYPE_TEXT: {
      Clay_TextRenderData *Text = &Command->renderData.text;
      char Buffer[4096 * 4];

      size_t Length = (size_t)Text->stringContents.length;
      if (Length >= sizeof(Buffer)) {
        Length = sizeof(Buffer) - 1;
      }

      memcpy(Buffer, Text->stringContents.chars, Length);
      Buffer[Length] = '\0';
      SDL_SetRenderDrawColor(Renderer, (Uint8)Text->textColor.r,
                             (Uint8)Text->textColor.g, (Uint8)Text->textColor.b,
                             (Uint8)Text->textColor.a);
      SDL_RenderDebugText(Renderer, Box.x, Box.y, Buffer);
      break;
    }

    case CLAY_RENDER_COMMAND_TYPE_BORDER:
    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START:
    case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END:
    case CLAY_RENDER_COMMAND_TYPE_NONE:
      // TODO: Implement these commands
      break;
    }
  }
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer("Mya", 1000, 700, SDL_WINDOW_RESIZABLE,
                                   &Window, &Renderer)) {
    fprintf(stderr, "SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());

    SDL_Quit();
    return 1;
  }
  SDL_SetRenderVSync(Renderer, 1);

  MyaElement *Page = MyaCreatePage();
  if (!Page) {
    fprintf(stderr, "Failed to create Mya page.\n");
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return 1;
  }

  uint32_t ClayMemorySize = Clay_MinMemorySize();
  void *ClayMemory = malloc(ClayMemorySize);

  if (!ClayMemory) {
    fprintf(stderr, "Failed to allocate Clay memory.\n");
    MyaDestroy(Page);
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return 1;
  }

  Clay_Arena Arena =
      Clay_CreateArenaWithCapacityAndMemory(ClayMemorySize, ClayMemory);

  int Width = 1000;
  int Height = 700;

  Clay_Initialize(
      Arena, (Clay_Dimensions){.width = (float)Width, .height = (float)Height},
      (Clay_ErrorHandler){.errorHandlerFunction = HandleClayError});

  Clay_SetMeasureTextFunction(MeasureText, NULL);

  bool Running = true;
  SDL_Event Event;

  while (Running) {
    while (SDL_PollEvent(&Event)) {
      if (Event.type == SDL_EVENT_QUIT) {
        Running = false;
      }
    }

    SDL_GetRenderOutputSize(Renderer, &Width, &Height);
    Clay_SetLayoutDimensions(
        (Clay_Dimensions){.width = (float)Width, .height = (float)Height});

    float MouseX, MouseY;
    SDL_MouseButtonFlags MouseButtons = SDL_GetMouseState(&MouseX, &MouseY);
    Clay_SetPointerState((Clay_Vector2){.x = MouseX, .y = MouseY},
                         (MouseButtons & SDL_BUTTON_LMASK) != 0);

    Clay_BeginLayout();

    CLAY_AUTO_ID(
        {.layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                               .height = CLAY_SIZING_GROW(0)},
                    .padding = {.left = 0, .right = 0, .top = 0, .bottom = 0},
                    .layoutDirection = CLAY_TOP_TO_BOTTOM},
         .backgroundColor = {25, 25, 30, 255}}) {
      MyaGenerateClayLayout(Page);
    }

    Clay_RenderCommandArray Commands = Clay_EndLayout(0);

    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
    SDL_RenderClear(Renderer);

    RenderClay(Commands);

    SDL_RenderPresent(Renderer);
  }

  MyaDestroy(Page);

  free(ClayMemory);

  SDL_DestroyRenderer(Renderer);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
