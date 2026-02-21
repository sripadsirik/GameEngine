#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include "ECS.h"
#include "Components.h"
#include "PhysicsSystem.h"

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

            if (transform.x < 0) {
                transform.x = 0;
                velocity.vx = 0;
            }
            if (transform.x > WORLD_WIDTH - 64) {
                transform.x = WORLD_WIDTH - 64;
                velocity.vx = 0;
            }
            if (transform.y > WORLD_HEIGHT - 64) {
                transform.y = WORLD_HEIGHT - 64;
                velocity.vy = 0;
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

void debugRenderColliders(ECS& ecs, SDL_Renderer* renderer, Camera& camera) {
    auto entities = ecs.getEntitiesWithComponent<Collider>();
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    
    for (Entity entity : entities) {
        if (ecs.hasComponent<Transform>(entity)) {
            auto& transform = ecs.getComponent<Transform>(entity);
            auto& collider = ecs.getComponent<Collider>(entity);
            
            int screenX = (int)(transform.x + collider.offsetX - camera.x);
            int screenY = (int)(transform.y + collider.offsetY - camera.y);
            
            SDL_Rect rect = {screenX, screenY, (int)collider.width, (int)collider.height};
            SDL_RenderDrawRect(renderer, &rect);
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
        "Game Engine - Phase 4: Physics",
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
    ecs.addComponent(player, Transform{400.0f, 100.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(player, Sprite{spriteTexture, 64, 64});
    ecs.addComponent(player, Velocity{0.0f, 0.0f});
    ecs.addComponent(player, Collider{64.0f, 64.0f, 0.0f, 0.0f, false});
    ecs.addComponent(player, RigidBody{1.0f, true, 1.0f, false});
    ecs.addComponent(player, PlayerController{300.0f, 500.0f, false});

    Entity ground = ecs.createEntity();
    ecs.addComponent(ground, Transform{0.0f, WORLD_HEIGHT - 100.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(ground, Collider{(float)WORLD_WIDTH, 100.0f, 0.0f, 0.0f, false});
    ecs.addComponent(ground, RigidBody{1.0f, false, 0.0f, true});

    Entity platform1 = ecs.createEntity();
    ecs.addComponent(platform1, Transform{300.0f, 800.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(platform1, Collider{400.0f, 50.0f, 0.0f, 0.0f, false});
    ecs.addComponent(platform1, RigidBody{1.0f, false, 0.0f, true});

    Entity platform2 = ecs.createEntity();
    ecs.addComponent(platform2, Transform{800.0f, 600.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(platform2, Collider{400.0f, 50.0f, 0.0f, 0.0f, false});
    ecs.addComponent(platform2, RigidBody{1.0f, false, 0.0f, true});

    Entity platform3 = ecs.createEntity();
    ecs.addComponent(platform3, Transform{1300.0f, 900.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(platform3, Collider{300.0f, 50.0f, 0.0f, 0.0f, false});
    ecs.addComponent(platform3, RigidBody{1.0f, false, 0.0f, true});

    for (int i = 0; i < 5; ++i) {
        Entity ball = ecs.createEntity();
        
        float randomX = 500.0f + (float)(rand() % 800);
        float randomY = 200.0f + (float)(rand() % 300);
        float randomScale = 0.5f + (float)(rand() % 100) / 100.0f;
        
        ecs.addComponent(ball, Transform{randomX, randomY, 0.0f, randomScale, randomScale});
        ecs.addComponent(ball, Sprite{spriteTexture, 64, 64});
        ecs.addComponent(ball, Velocity{0.0f, 0.0f});
        ecs.addComponent(ball, Collider{64.0f * randomScale, 64.0f * randomScale, 0.0f, 0.0f, false});
        ecs.addComponent(ball, RigidBody{1.0f, true, 1.0f, false});
    }

    bool isRunning = true;
    SDL_Event event;

    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    Camera camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    Uint32 lastTime = SDL_GetTicks();

    bool showColliders = true;

    while (isRunning) {
        frameStart = SDL_GetTicks();
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                if (event.key.keysym.sym == SDLK_c) {
                    showColliders = !showColliders;
                }
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        groundDetectionSystem(ecs);
        playerControllerSystem(ecs, deltaTime, keystate);
        gravitySystem(ecs, deltaTime);
        movementSystem(ecs, deltaTime);
        physicsSystem(ecs, deltaTime);

        auto& playerTransform = ecs.getComponent<Transform>(player);
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

        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        auto platforms = ecs.getEntitiesWithComponent<RigidBody>();
        for (Entity entity : platforms) {
            if (ecs.hasComponent<Collider>(entity) && !ecs.hasComponent<Sprite>(entity)) {
                auto& transform = ecs.getComponent<Transform>(entity);
                auto& collider = ecs.getComponent<Collider>(entity);
                
                int screenX = (int)(transform.x - camera.x);
                int screenY = (int)(transform.y - camera.y);
                
                SDL_Rect rect = {screenX, screenY, (int)collider.width, (int)collider.height};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        renderSystem(ecs, renderer, camera);

        if (showColliders) {
            debugRenderColliders(ecs, renderer, camera);
        }

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