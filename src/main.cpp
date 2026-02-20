// =============================================================================
// main.cpp - Application Entry Point and Core Game Loop (Phase 1)
// =============================================================================
//
// This file serves as the entry point for the GameEngine application and
// contains the complete Phase 1 implementation of the engine's core runtime.
// It bootstraps SDL2, creates the window and hardware-accelerated renderer,
// runs the main game loop, and tears everything down cleanly on exit.
//
// PHASE 1 SCOPE:
//   Phase 1 establishes the foundational scaffolding that all future engine
//   systems will build upon. At this stage the engine proves that it can open
//   a window, drive a fixed-timestep game loop at 60 FPS, respond to OS
//   events, and issue draw calls to the GPU — nothing more, nothing less.
//   Game logic, entity systems, asset loading, audio, and input abstractions
//   are deliberately deferred to later phases.
//
// ARCHITECTURE OVERVIEW:
//   The file is structured as a linear startup/shutdown sequence wrapped
//   around a three-phase game loop:
//
//   1. INITIALIZATION
//      SDL2 is initialised with video support. A window (800x600, centered,
//      titled "Game Engine - Phase 1") and a hardware-accelerated SDL_Renderer
//      are created. If any step fails the program prints the SDL error and
//      exits early, cleaning up any resources that were successfully acquired.
//
//   2. GAME LOOP  (runs until isRunning becomes false)
//      a) Event Handling  — SDL_PollEvent drains the OS event queue each
//         frame. SDL_QUIT (window X button) and SDLK_ESCAPE both set
//         isRunning = false, which terminates the loop on the next iteration.
//      b) Update          — Reserved for game-state mutation (physics, AI,
//         animations, etc.). Currently a no-op placeholder.
//      c) Render          — The back-buffer is cleared to a dark blue/navy
//         colour (RGB 30,30,46). A red test rectangle (100x100 px) is drawn
//         centred near the middle of the window to confirm rendering works,
//         then SDL_RenderPresent flips the back-buffer to the screen.
//      d) Frame Limiter   — SDL_GetTicks measures how long the frame took. If
//         the frame completed faster than 16 ms (60 FPS target), SDL_Delay
//         sleeps the remainder so the loop runs at a stable 60 Hz without
//         pegging the CPU.
//
//   3. CLEANUP
//      SDL_DestroyRenderer, SDL_DestroyWindow, and SDL_Quit are called in
//      reverse-creation order to release all SDL resources before the process
//      exits with code 0.
//
// DEPENDENCIES:
//   - SDL2  (linked via CMake; DLL copied to build dir by post-build step)
//   - C++ standard library <iostream> for error reporting
//
// FUTURE WORK:
//   - Extract window/renderer creation into an Engine or Application class
//   - Replace raw SDL_Delay frame limiter with a proper delta-time system
//   - Add an input manager to abstract SDL_Event processing
//   - Introduce a Scene/Entity system for game-object management
//   - Integrate an asset pipeline (textures, audio, fonts)
// =============================================================================

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
#undef main
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game Engine - Phase 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool isRunning = true;
    SDL_Event event;

    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    int squareX = 350;
    int squareY = 250;


    while (isRunning) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) squareX += 5;
                if (event.key.keysym.sym == SDLK_LEFT) squareX -= 5;
                if (event.key.keysym.sym == SDLK_DOWN) squareY += 5;
                if (event.key.keysym.sym == SDLK_UP) squareY -= 5;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);
        SDL_RenderClear(renderer);

        SDL_Rect testRect = {squareX, squareY, 100, 100};
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        SDL_RenderFillRect(renderer, &testRect);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
