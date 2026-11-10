#include "Mya.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Clay.h"

extern MyaElement *MyaCreatePage(void);

SDL_Window *Window;
SDL_Renderer *Renderer;

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

int main(int Argc, char **Argv) {
  (void)Argc;
  (void)Argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

  if (!TTF_Init()) {
    fprintf(stderr, "TTF_Init: %s\n", SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer("Mya", 800, 600, SDL_WINDOW_RESIZABLE,
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
  MyaApplyStyles(Page);

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

  int Width = 800;
  int Height = 600;
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

    MyaUnitContext RootContext = {.FontSize = 16.0f,
                                  .RootFontSize = 16.0f,
                                  .ViewportWidth = Width,
                                  .ViewportHeight = Height,
                                  .ReferenceWidth = Width,
                                  .ReferenceHeight = Height};

    CLAY_AUTO_ID(
        {.layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                               .height = CLAY_SIZING_GROW(0)},
                    .padding = {.left = 0, .right = 0, .top = 0, .bottom = 0},
                    .layoutDirection = CLAY_TOP_TO_BOTTOM},
         .backgroundColor = {255, 255, 255, 255}}) {
      MyaGenerateClayLayout(Page, &RootContext);
    }

    Clay_RenderCommandArray Commands = Clay_EndLayout(0);

    MyaUpdateFonts();
    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
    SDL_RenderClear(Renderer);
    MyaRenderClay(Commands);
    SDL_RenderPresent(Renderer);
  }

  MyaDestroy(Page);

  free(ClayMemory);

  SDL_DestroyRenderer(Renderer);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
