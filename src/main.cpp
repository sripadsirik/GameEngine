#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <cmath>
#include "ECS.h"
#include "Components.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include <SDL_ttf.h>

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
            
            SDL_Rect srcRect = {sprite.srcX, sprite.srcY, sprite.srcWidth, sprite.srcHeight};
            
            if (sprite.srcWidth == 0 || sprite.srcHeight == 0) {
                SDL_RenderCopyEx(
                    renderer,
                    (SDL_Texture*)sprite.texture,
                    NULL,
                    &destRect,
                    transform.rotation,
                    NULL,
                    SDL_FLIP_NONE
                );
            } else {
                SDL_RenderCopyEx(
                    renderer,
                    (SDL_Texture*)sprite.texture,
                    &srcRect,
                    &destRect,
                    transform.rotation,
                    NULL,
                    SDL_FLIP_NONE
                );
            }
        }
    }
}

void renderParticles(ECS& ecs, SDL_Renderer* renderer, Camera& camera) {
    auto particles = ecs.getEntitiesWithComponent<Particle>();
    
    for (Entity entity : particles) {
        if (ecs.hasComponent<Transform>(entity)) {
            auto& transform = ecs.getComponent<Transform>(entity);
            auto& particle = ecs.getComponent<Particle>(entity);
            
            int screenX = (int)(transform.x - camera.x);
            int screenY = (int)(transform.y - camera.y);
            
            SDL_SetRenderDrawColor(renderer, 
                particle.colorR, 
                particle.colorG, 
                particle.colorB, 
                particle.colorA);
            
            SDL_Rect rect = {screenX, screenY, 8, 8};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface == nullptr) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderHealthBar(SDL_Renderer* renderer, int x, int y, int width, int height, int current, int max) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    float healthPercent = (float)current / (float)max;
    int healthWidth = (int)(width * healthPercent);
    
    if (healthPercent > 0.6f) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else if (healthPercent > 0.3f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    
    SDL_Rect healthRect = {x, y, healthWidth, height};
    SDL_RenderFillRect(renderer, &healthRect);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game Engine - Phase 5: Complete",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("assets/PressStart2P-Regular.ttf", 14);
    if (font == nullptr) {
        std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }

    const int DAMAGE_CHANNEL = 0;
    const int HEAL_CHANNEL   = 1;

    Mix_Chunk* damageSound = Mix_LoadWAV("assets/sounds/845959__josefpres__piano-loops-205-octave-long-loop-120-bpm.wav");
    if (damageSound == nullptr) {
        std::cout << "Failed to load damage sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(damageSound, 64);   // softer, ~50% volume for damage
    }

    Mix_Chunk* healSound = Mix_LoadWAV("assets/sounds/845959__josefpres__piano-loops-205-octave-long-loop-120-bpm.wav");
    if (healSound == nullptr) {
        std::cout << "Failed to load heal sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_VolumeChunk(healSound, 128);    // full volume for heal
    }

    SDL_Surface* loadedSurface = IMG_Load("assets/test_sprite.png");
    if (loadedSurface == nullptr) {
        std::cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* spriteTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);

    if (spriteTexture == nullptr) {
        std::cout << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    ECS ecs;

    Entity player = ecs.createEntity();
    ecs.addComponent(player, Transform{400.0f, 100.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(player, Sprite{spriteTexture, 64, 64, 0, 0, 0, 0});
    ecs.addComponent(player, Velocity{0.0f, 0.0f});
    ecs.addComponent(player, Collider{64.0f, 64.0f, 0.0f, 0.0f, false});
    ecs.addComponent(player, RigidBody{1.0f, true, 1.0f, false});
    ecs.addComponent(player, PlayerController{300.0f, 500.0f, false});
    ecs.addComponent(player, Health{100, 100});

    Entity playerParticles = ecs.createEntity();
    ecs.addComponent(playerParticles, Transform{400.0f, 100.0f, 0.0f, 1.0f, 1.0f});
    ecs.addComponent(playerParticles, ParticleEmitter{
        20.0f, 0.5f, 0.0f, 50, true,
        -50.0f, 50.0f, -100.0f, -50.0f
    });

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
        ecs.addComponent(ball, Sprite{spriteTexture, 64, 64, 0, 0, 0, 0});
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

    bool showColliders = false;
    
    int frameCount = 0;
    float fpsTimer = 0.0f;
    int currentFPS = 0;

    while (isRunning) {
        frameStart = SDL_GetTicks();
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastTime = currentTime;

        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            currentFPS = frameCount;
            frameCount = 0;
            fpsTimer = 0.0f;
        }

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
                if (event.key.keysym.sym == SDLK_h && event.key.repeat == 0) {
                    auto& playerHealth = ecs.getComponent<Health>(player);
                    playerHealth.current -= 10;
                    if (playerHealth.current < 0) playerHealth.current = 0;
                    if (damageSound != nullptr) {
                        Mix_HaltChannel(DAMAGE_CHANNEL);
                        Mix_PlayChannel(DAMAGE_CHANNEL, damageSound, 0);
                    }
                }
                if (event.key.keysym.sym == SDLK_j && event.key.repeat == 0) {
                    auto& playerHealth = ecs.getComponent<Health>(player);
                    playerHealth.current += 10;
                    if (playerHealth.current > playerHealth.max) playerHealth.current = playerHealth.max;
                    if (healSound != nullptr) {
                        Mix_HaltChannel(HEAL_CHANNEL);
                        Mix_PlayChannel(HEAL_CHANNEL, healSound, 0);
                    }
                }
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        groundDetectionSystem(ecs);
        playerControllerSystem(ecs, deltaTime, keystate);
        gravitySystem(ecs, deltaTime);
        movementSystem(ecs, deltaTime);
        physicsSystem(ecs, deltaTime);
        animationSystem(ecs, deltaTime);
        particleSystem(ecs, deltaTime);
        lifetimeSystem(ecs, deltaTime);

        auto& playerTransform = ecs.getComponent<Transform>(player);
        auto& particleEmitterTransform = ecs.getComponent<Transform>(playerParticles);
        particleEmitterTransform.x = playerTransform.x + 32;
        particleEmitterTransform.y = playerTransform.y + 64;

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

        renderParticles(ecs, renderer, camera);
        renderSystem(ecs, renderer, camera);

        if (showColliders) {
            debugRenderColliders(ecs, renderer, camera);
        }

        if (font != nullptr) {
            auto& playerHealth = ecs.getComponent<Health>(player);
            
            renderHealthBar(renderer, 10, 10, 200, 20, playerHealth.current, playerHealth.max);
            
            char healthText[32];
            sprintf(healthText, "HP: %d/%d", playerHealth.current, playerHealth.max);
            SDL_Color white = {255, 255, 255, 255};
            renderText(renderer, font, healthText, 10, 35, white);
            
            char fpsText[32];
            sprintf(fpsText, "FPS: %d", currentFPS);
            renderText(renderer, font, fpsText, 10, 65, white);
            
            char posText[64];
            sprintf(posText, "Pos: (%.0f, %.0f)", playerTransform.x, playerTransform.y);
            renderText(renderer, font, posText, 10, 95, white);
            
            renderText(renderer, font, "H - Damage  J - Heal", 10, SCREEN_HEIGHT - 30, white);
        }

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyTexture(spriteTexture);
    TTF_CloseFont(font);
    Mix_FreeChunk(damageSound);
    Mix_FreeChunk(healSound);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}