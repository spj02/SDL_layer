#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_layer.h>

int main(void) {
  int status = 1;
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_PropertiesID props;

  // Initialize SDL3 and SDL3_layer.
  if (!SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland")) {
    fprintf(stderr, "error: SDL_SetHint() failed: %s\n", SDL_GetError());
    goto return_status;
  }
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "error: SDL_Init() failed: %s\n", SDL_GetError());
    goto return_status;
  }
  if (!LYR_Init()) {
    fprintf(stderr, "error: LYR_Init() failed: %s\n", SDL_GetError());
    goto quit_sdl;
  }

  // Create layer window and renderer.
  if ((props = SDL_CreateProperties()) == 0) {
    fprintf(stderr, "error: SDL_CreateProperties() failed: %s\n", SDL_GetError());
    goto quit_layer;
  }
  if (
    !SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 512) ||
    !SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 512) ||
    !SDL_SetNumberProperty(props, LYR_PROP_WINDOW_CREATE_LAYER_NUMBER, LYR_LAYER_OVERLAY)
  ) {
    SDL_DestroyProperties(props);
    fprintf(stderr, "error: SDL_SetNumberProperty() failed: %s\n", SDL_GetError());
    goto destroy_props;
  }
  if ((window = LYR_CreateWindow(props)) == NULL) {
    fprintf(stderr, "error: LYR_CreateWindow() failed: %s\n", SDL_GetError());
    goto destroy_props;
  }
  if ((renderer = SDL_CreateRenderer(window, "gpu,software")) == NULL) {
    fprintf(stderr, "error: SDL_CreateRenderer() failed: %s\n", SDL_GetError());
    goto destroy_window;
  }

  // Process events and paint a red background.
  for (;;) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          status = 0;
          goto destroy_renderer;
      }
    }
    if (!SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128)) {
      fprintf(stderr, "error: SDL_SetRenderDrawColor() failed: %s\n", SDL_GetError());
    }
    if (!SDL_RenderClear(renderer)) {
      fprintf(stderr, "error: SDL_RenderClear() failed: %s\n", SDL_GetError());
    }
    if (!SDL_RenderPresent(renderer)) {
      fprintf(stderr, "error: SDL_RenderPresent() failed: %s\n", SDL_GetError());
    }
  }

destroy_renderer:
  SDL_DestroyRenderer(renderer);
destroy_window:
  SDL_DestroyWindow(window);
destroy_props:
  SDL_DestroyProperties(props);
quit_layer:
  LYR_Quit();
quit_sdl:
  SDL_Quit();
return_status:
  return status;
}
