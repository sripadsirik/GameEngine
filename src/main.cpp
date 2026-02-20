#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
#undef main
int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Game Engine - Phase 1",           // Window title
        SDL_WINDOWPOS_CENTERED,             // X position (centered)
        SDL_WINDOWPOS_CENTERED,             // Y position (centered)
        SCREEN_WIDTH,                       // Width
        SCREEN_HEIGHT,                      // Height
        SDL_WINDOW_SHOWN                    // Flags (make it visible)
    );

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create renderer (this draws to the window)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,                             // Which window to render to
        -1,                                 // Index of rendering driver (-1 = first supporting flags)
        SDL_RENDERER_ACCELERATED            // Use hardware acceleration
    );

    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Game loop control
    bool isRunning = true;
    SDL_Event event;

    // Frame timing
    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;  // milliseconds per frame
    Uint32 frameStart;
    int frameTime;

    // GAME LOOP
    while (isRunning) {
        frameStart = SDL_GetTicks();  // Get current time in milliseconds

        // EVENT HANDLING - Process all pending events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;  // User closed the window
            }
            
            // Check for ESC key press
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
        }

        // UPDATE GAME STATE
        // (Nothing to update yet, but this is where game logic goes)

        // RENDER
        // Clear screen with a color (R, G, B, Alpha)
        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);  // Dark blue background
        SDL_RenderClear(renderer);

        // Draw a rectangle to test rendering
        SDL_Rect testRect = {350, 250, 100, 100};  // x, y, width, height
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);  // Red color
        SDL_RenderFillRect(renderer, &testRect);

        // Present the rendered frame
        SDL_RenderPresent(renderer);

        // FRAME RATE CONTROL
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);  // Wait to maintain 60 FPS
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}