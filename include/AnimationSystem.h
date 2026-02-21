#pragma once
#include "ECS.h"
#include "Components.h"
#include <SDL.h>

void animationSystem(ECS& ecs, float deltaTime) {
    auto entities = ecs.getEntitiesWithComponent<Animation>();

    for (Entity entity : entities) {
        if (!ecs.hasComponent<Sprite>(entity)) continue;

        auto& animation = ecs.getComponent<Animation>(entity);
        auto& sprite = ecs.getComponent<Sprite>(entity);

        if (!animation.playing) continue;

        animation.elapsedTime += deltaTime;

        if (animation.elapsedTime >= animation.frameTime) {
            animation.elapsedTime = 0.0f;
            animation.currentFrame++;

            if (animation.currentFrame >= animation.totalFrames) {
                if (animation.loop) {
                    animation.currentFrame = 0;
                } else {
                    animation.currentFrame = animation.totalFrames - 1;
                    animation.playing = false;
                }
            }
        }

        sprite.texture = animation.spriteSheet;
        sprite.srcX = animation.currentFrame * animation.frameWidth;
        sprite.srcY = 0;
        sprite.srcWidth = animation.frameWidth;
        sprite.srcHeight = animation.frameHeight;
    }
}

void particleSystem(ECS& ecs, float deltaTime) {
    auto emitters = ecs.getEntitiesWithComponent<ParticleEmitter>();

    for (Entity emitter : emitters) {
        if (!ecs.hasComponent<Transform>(emitter)) continue;

        auto& particleEmitter = ecs.getComponent<ParticleEmitter>(emitter);
        auto& emitterTransform = ecs.getComponent<Transform>(emitter);

        if (!particleEmitter.active) continue;

        particleEmitter.timeSinceLastEmit += deltaTime;

        if (particleEmitter.timeSinceLastEmit >= 1.0f / particleEmitter.emissionRate) {
            particleEmitter.timeSinceLastEmit = 0.0f;

            Entity particle = ecs.createEntity();

            float vx = particleEmitter.minVelocityX + 
                      (float)(rand() % 100) / 100.0f * 
                      (particleEmitter.maxVelocityX - particleEmitter.minVelocityX);
            float vy = particleEmitter.minVelocityY + 
                      (float)(rand() % 100) / 100.0f * 
                      (particleEmitter.maxVelocityY - particleEmitter.minVelocityY);

            ecs.addComponent(particle, Transform{
                emitterTransform.x, 
                emitterTransform.y, 
                0.0f, 
                0.3f, 
                0.3f
            });
            ecs.addComponent(particle, Velocity{vx, vy});
            ecs.addComponent(particle, Particle{
                particleEmitter.particleLifetime,
                0.0f,
                255, 200, 100, 255
            });
        }
    }

    auto particles = ecs.getEntitiesWithComponent<Particle>();
    std::vector<Entity> toDestroy;

    for (Entity entity : particles) {
        if (!ecs.hasComponent<Transform>(entity)) continue;

        auto& particle = ecs.getComponent<Particle>(entity);
        
        particle.age += deltaTime;

        if (particle.age >= particle.lifetime) {
            toDestroy.push_back(entity);
            continue;
        }

        float lifeRatio = particle.age / particle.lifetime;
        particle.colorA = (int)(255 * (1.0f - lifeRatio));
    }

    for (Entity entity : toDestroy) {
        ecs.destroyEntity(entity);
    }
}

void lifetimeSystem(ECS& ecs, float deltaTime) {
    auto entities = ecs.getEntitiesWithComponent<Lifetime>();
    std::vector<Entity> toDestroy;

    for (Entity entity : entities) {
        auto& lifetime = ecs.getComponent<Lifetime>(entity);
        lifetime.elapsed += deltaTime;

        if (lifetime.elapsed >= lifetime.duration) {
            toDestroy.push_back(entity);
        }
    }

    for (Entity entity : toDestroy) {
        ecs.destroyEntity(entity);
    }
}