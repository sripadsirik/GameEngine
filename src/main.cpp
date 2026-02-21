#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include "ECS.h"
#include "Components.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int WORLD_WIDTH = 2000;
const int WORLD_HEIGHT = 1500;

struct Camera {
    float x;
    float y;
    int width;
    int height;
};

void updateCamera(Camera& camera, float targetX, float targetY) {
    camera.x = targetX - camera.width / 2;
    camera.y = targetY - camera.height / 2;

    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > WORLD_WIDTH - camera.width) camera.x = WORLD_WIDTH - camera.width;
    if (camera.y > WORLD_HEIGHT - camera.height) camera.y = WORLD_HEIGHT - camera.height;
}

void movementSystem(ECS& ecs, float deltaTime) {
    auto entities = ecs.getEntitiesWithComponent<Velocity>();
    
    for (Entity entity : entities) {
        if (ecs.hasComponent<Transform>(entity)) {
            auto& transform = ecs.getComponent<Transform>(entity);
            auto& velocity = ecs.getComponent<Velocity>(entity);
            
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;

            if (transform.x < 0 || transform.x > WORLD_WIDTH - 64) {
                velocity.vx = -velocity.vx;
                transform.x = std::max(0.0f, std::min(transform.x, (float)(WORLD_WIDTH - 64)));
            }
            if (transform.y < 0 || transform.y > WORLD_HEIGHT - 64) {
                velocity.vy = -velocity.vy;
                transform.y = std::max(0.0f, std::min(transform.y, (float)(WORLD_HEIGHT - 64)));
            }
        }
    }
}

void renderSystem(ECS& ecs, SDL_Renderer* renderer, Camera& camera) {
    auto entities = ecs.getEntitiesWithComponent<Sprite>();
    
    for (Entity entity : entities) {
        if (ecs.hasComponent<Transform>(entity)) {
            auto& transform = ecs.getComponent<Transform>(entity);
            auto& sprite = ecs.getComponent<Sprite>(entity);
            
            int screenX = (int)(transform.x - camera.x);
            int screenY = (int)(transform.y - camera.y);
            
            int scaledWidth = (int)(sprite.width * transform.scaleX);
            int scaledHeight = (int)(sprite.height * transform.scaleY);
            
            SDL_Rect destRect = {screenX, screenY, scaledWidth, scaledHeight};
            
            SDL_RenderCopyEx(
                renderer,
                (SDL_Texture*)sprite.texture,
                NULL,
                &destRect,
                transform.rotation,
                NULL,
                SDL_FLIP_NONE
            );
        }
    }
}

#undef main
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game Engine - Phase 3: ECS",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Surface* loadedSurface = IMG_Load("assets/test_sprite.png");
    if (loadedSurface == nullptr) {
        std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* spriteTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);

    if (spriteTexture == nullptr) {
        std::cout << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    ECS ecs;

    Entity player = ecs.createEntity();
    ecs.addComponent(player, Transform{WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(player, Sprite{spriteTexture, 64, 64});

    for (int i = 0; i < 10; ++i) {
        Entity bouncer = ecs.createEntity();
        
        float randomX = (float)(rand() % WORLD_WIDTH);
        float randomY = (float)(rand() % WORLD_HEIGHT);
        float randomVX = (float)(rand() % 200 - 100);
        float randomVY = (float)(rand() % 200 - 100);
        float randomScale = 0.5f + (float)(rand() % 150) / 100.0f;
        
        ecs.addComponent(bouncer, Transform{randomX, randomY, 0.0f, randomScale, randomScale});
        ecs.addComponent(bouncer, Sprite{spriteTexture, 64, 64});
        ecs.addComponent(bouncer, Velocity{randomVX, randomVY});
    }

    bool isRunning = true;
    SDL_Event event;

    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    float spriteSpeed = 200.0f;
    Camera camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    Uint32 lastTime = SDL_GetTicks();

    while (isRunning) {
        frameStart = SDL_GetTicks();
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        
        auto& playerTransform = ecs.getComponent<Transform>(player);
        
        if (keystate[SDL_SCANCODE_RIGHT]) playerTransform.x += spriteSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_LEFT]) playerTransform.x -= spriteSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_DOWN]) playerTransform.y += spriteSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_UP]) playerTransform.y -= spriteSpeed * deltaTime;

        if (playerTransform.x < 0) playerTransform.x = 0;
        if (playerTransform.y < 0) playerTransform.y = 0;
        if (playerTransform.x > WORLD_WIDTH - 64) playerTransform.x = WORLD_WIDTH - 64;
        if (playerTransform.y > WORLD_HEIGHT - 64) playerTransform.y = WORLD_HEIGHT - 64;

        movementSystem(ecs, deltaTime);

        updateCamera(camera, playerTransform.x + 32, playerTransform.y + 32);

        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 80, 80, 100, 255);
        for (int x = 0; x < WORLD_WIDTH; x += 100) {
            SDL_Rect line = {x - (int)camera.x, 0, 2, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &line);
        }
        for (int y = 0; y < WORLD_HEIGHT; y += 100) {
            SDL_Rect line = {0, y - (int)camera.y, SCREEN_WIDTH, 2};
            SDL_RenderFillRect(renderer, &line);
        }

        renderSystem(ecs, renderer, camera);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyTexture(spriteTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}