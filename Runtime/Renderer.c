#include "Mya.h"

#include "Clay.h"
#include <SDL3/SDL.h>
#include <stdio.h>

MyaFontEntry *Fonts = NULL;
int FontCount = 0;

void MyaUpdateFonts() {
  for (int Idx = 0; Idx < FontCount; Idx++) {
    MyaFontEntry *FontEntry = &Fonts[Idx];

    if (!FontEntry->Font) {
      FontEntry->Font =
          TTF_OpenFont(FontEntry->FontName, (float)FontEntry->FontSize);
      if (!FontEntry->Font) {
        fprintf(stderr, "Failed to load font '%s': %s\n", FontEntry->FontName,
                SDL_GetError());
      }
    }
  }
}

void MyaRenderClay(Clay_RenderCommandArray Commands) {
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

      SDL_Color Color = {(Uint8)Text->textColor.r, (Uint8)Text->textColor.g,
                         (Uint8)Text->textColor.b, (Uint8)Text->textColor.a};

      TTF_Font *Font = Fonts[Text->fontId].Font;

      SDL_Surface *Surface =
          TTF_RenderText_Blended(Font, Buffer, Length, Color);
      SDL_Texture *Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
      SDL_DestroySurface(Surface);

      SDL_FRect DestinationRectangle = {Box.x, Box.y, Box.width, Box.height};
      SDL_RenderTexture(Renderer, Texture, NULL, &DestinationRectangle);
      SDL_DestroyTexture(Texture);
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
