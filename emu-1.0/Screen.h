
#pragma once

#include <SDL.h>
#include <cstdint>

class Screen {
public:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  bool closed = false;
  bool debug = false;
  uint32_t drawcounter = 0;
  
  int SCALING = 6;

  ~Screen() {
    destroy();
  }

  void init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
      throw std::runtime_error("Unable to initalize SDL2");
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    window = SDL_CreateWindow(
      "EMU1.0",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      64 * SCALING, 64 * SCALING,
      SDL_WINDOW_SHOWN
    );

    if (!window) {
      throw std::runtime_error("Failed to create window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
      throw std::runtime_error("Failed to create renderer");
    }

    SDL_RenderSetScale(renderer, SCALING, SCALING);
    clear();
    SDL_RenderPresent(renderer);
  }

  void tick() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      handleEvent(event);
      //std::cerr << "handled event" << std::endl;
    }

/*    if (drawcounter++ >= 10) {
      SDL_RenderPresent(renderer);
      drawcounter = 0;
    }*/

    //SDL_Delay(1000 / 60);
    SDL_RenderPresent(renderer);
  }

  void handleEvent(const SDL_Event& event) {
    switch (event.type) {
      case SDL_QUIT:
        closed = true;
        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_D:
          if (event.key.state) {
            debug = !debug;
          }

          break;
        default:
          break;
        }
        //handleKeyboardEvent(event.key);
        break;
      case SDL_MOUSEMOTION:
        //handleMouseMotionEvent(event.motion);
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        //handleMouseButtonEvent(event.button);
        break;
      case SDL_WINDOWEVENT:
        //handleWindowEvent(event.window);
        break;
      default:
        break;
    }
  }

  void destroy() {
    if (window) {
      SDL_DestroyWindow(window);
      window = nullptr;
    }

    if (renderer) {
      SDL_DestroyRenderer(renderer);
      renderer = nullptr;
    }
  }

  void clear() {
    //std::cerr << "clear" << std::endl;
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  }

  void draw(int8_t x, int8_t y, uint8_t r, uint8_t g, uint8_t b) {
    //std::cerr << "draw " << (int)x << " " << (int)y << " " << (int)r << std::endl;
    SDL_Rect crect = {x, y, 1, 1};

    SDL_SetRenderDrawColor(renderer, r, g, b, 255); // the rect color (solid red)
    //for (int i = 0; i < 8; i++) {
      //if (line & (1 << i)) {
        SDL_RenderFillRect(renderer, &crect);
      //}

      //crect.x--;
    //}
  }
};
