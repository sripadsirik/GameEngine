#pragma once
#include "ECS.h"
#include "Components.h"
#include <vector>
#include <algorithm>

struct AABB {
    float x, y, width, height;
};

struct CollisionPair {
    Entity a;
    Entity b;
};

AABB getAABB(const Transform& transform, const Collider& collider) {
    return AABB{
        transform.x + collider.offsetX,
        transform.y + collider.offsetY,
        collider.width,
        collider.height
    };
}

bool checkAABBCollision(const AABB& a, const AABB& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

void resolveCollision(Transform& transformA, Velocity& velocityA, const Collider& colliderA,
                     Transform& transformB, Velocity& velocityB, const Collider& colliderB,
                     bool isStaticB) {
    AABB boxA = getAABB(transformA, colliderA);
    AABB boxB = getAABB(transformB, colliderB);

    float overlapX = std::min(boxA.x + boxA.width, boxB.x + boxB.width) - std::max(boxA.x, boxB.x);
    float overlapY = std::min(boxA.y + boxA.height, boxB.y + boxB.height) - std::max(boxA.y, boxB.y);

    if (overlapX < overlapY) {
        if (boxA.x < boxB.x) {
            if (isStaticB) {
                transformA.x -= overlapX;
            } else {
                transformA.x -= overlapX / 2;
                transformB.x += overlapX / 2;
            }
        } else {
            if (isStaticB) {
                transformA.x += overlapX;
            } else {
                transformA.x += overlapX / 2;
                transformB.x -= overlapX / 2;
            }
        }
        
        velocityA.vx = -velocityA.vx * 0.5f;
        if (!isStaticB) velocityB.vx = -velocityB.vx * 0.5f;
    } else {
        if (boxA.y < boxB.y) {
            // A is above B (landing on top)
            if (isStaticB) {
                transformA.y -= overlapY;
                if (velocityA.vy > 0) velocityA.vy = 0;
            } else {
                transformA.y -= overlapY / 2;
                transformB.y += overlapY / 2;
                velocityA.vy = -velocityA.vy * 0.5f;
                velocityB.vy = -velocityB.vy * 0.5f;
            }
        } else {
            // A is below B (hitting a ceiling)
            if (isStaticB) {
                transformA.y += overlapY;
                if (velocityA.vy < 0) velocityA.vy = 0;
            } else {
                transformA.y += overlapY / 2;
                transformB.y -= overlapY / 2;
                velocityA.vy = -velocityA.vy * 0.5f;
                velocityB.vy = -velocityB.vy * 0.5f;
            }
        }
    }
}

void gravitySystem(ECS& ecs, float deltaTime) {
    auto entities = ecs.getEntitiesWithComponent<RigidBody>();
    const float GRAVITY = 980.0f;

    for (Entity entity : entities) {
        if (!ecs.hasComponent<Velocity>(entity)) continue;
        
        auto& rigidBody = ecs.getComponent<RigidBody>(entity);
        
        if (rigidBody.useGravity && !rigidBody.isStatic) {
            auto& velocity = ecs.getComponent<Velocity>(entity);
            velocity.vy += GRAVITY * rigidBody.gravityScale * deltaTime;
            const float MAX_FALL_SPEED = 900.0f;
            if (velocity.vy > MAX_FALL_SPEED) velocity.vy = MAX_FALL_SPEED;
        }
    }
}

void physicsSystem(ECS& ecs, float deltaTime) {
    auto entities = ecs.getEntitiesWithComponent<Collider>();
    std::vector<CollisionPair> collisions;

    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            Entity entityA = entities[i];
            Entity entityB = entities[j];

            if (!ecs.hasComponent<Transform>(entityA) || !ecs.hasComponent<Transform>(entityB)) {
                continue;
            }

            auto& transformA = ecs.getComponent<Transform>(entityA);
            auto& transformB = ecs.getComponent<Transform>(entityB);
            auto& colliderA = ecs.getComponent<Collider>(entityA);
            auto& colliderB = ecs.getComponent<Collider>(entityB);

            AABB boxA = getAABB(transformA, colliderA);
            AABB boxB = getAABB(transformB, colliderB);

            if (checkAABBCollision(boxA, boxB)) {
                if (colliderA.isTrigger || colliderB.isTrigger) {
                    continue;
                }

                bool hasVelocityA = ecs.hasComponent<Velocity>(entityA);
                bool hasVelocityB = ecs.hasComponent<Velocity>(entityB);

                if (hasVelocityA && hasVelocityB) {
                    auto& velocityA = ecs.getComponent<Velocity>(entityA);
                    auto& velocityB = ecs.getComponent<Velocity>(entityB);
                    
                    bool isStaticB = false;
                    if (ecs.hasComponent<RigidBody>(entityB)) {
                        isStaticB = ecs.getComponent<RigidBody>(entityB).isStatic;
                    }

                    resolveCollision(transformA, velocityA, colliderA, 
                                   transformB, velocityB, colliderB, isStaticB);
                } else if (hasVelocityA && !hasVelocityB) {
                    auto& velocityA = ecs.getComponent<Velocity>(entityA);
                    Velocity dummyVelocity = {0, 0};

                    resolveCollision(transformA, velocityA, colliderA,
                                   transformB, dummyVelocity, colliderB, true);
                } else if (!hasVelocityA && hasVelocityB) {
                    // Static entity is A, dynamic entity is B — swap roles
                    auto& velocityB = ecs.getComponent<Velocity>(entityB);
                    Velocity dummyVelocity = {0, 0};

                    resolveCollision(transformB, velocityB, colliderB,
                                   transformA, dummyVelocity, colliderA, true);
                }
            }
        }
    }
}

void playerControllerSystem(ECS& ecs, float deltaTime, const Uint8* keystate) {
    auto entities = ecs.getEntitiesWithComponent<PlayerController>();

    for (Entity entity : entities) {
        if (!ecs.hasComponent<Transform>(entity) || !ecs.hasComponent<Velocity>(entity)) {
            continue;
        }

        auto& controller = ecs.getComponent<PlayerController>(entity);
        auto& velocity = ecs.getComponent<Velocity>(entity);

        velocity.vx = 0;

        if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
            velocity.vx = controller.speed;
        }
        if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
            velocity.vx = -controller.speed;
        }

        if (keystate[SDL_SCANCODE_SPACE] || keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W]) {
            if (controller.isGrounded) {
                velocity.vy = -controller.jumpForce;
                controller.isGrounded = false;
            }
        }
    }
}

void groundDetectionSystem(ECS& ecs) {
    auto players = ecs.getEntitiesWithComponent<PlayerController>();

    for (Entity player : players) {
        if (!ecs.hasComponent<Transform>(player) || 
            !ecs.hasComponent<Collider>(player) ||
            !ecs.hasComponent<Velocity>(player)) {
            continue;
        }

        auto& controller = ecs.getComponent<PlayerController>(player);
        auto& playerTransform = ecs.getComponent<Transform>(player);
        auto& playerCollider = ecs.getComponent<Collider>(player);
        auto& playerVelocity = ecs.getComponent<Velocity>(player);

        controller.isGrounded = false;

        AABB playerBox = getAABB(playerTransform, playerCollider);
        
        AABB groundCheckBox;
        groundCheckBox.x = playerBox.x + 5;
        groundCheckBox.y = playerBox.y + playerBox.height;
        groundCheckBox.width = playerBox.width - 10;
        groundCheckBox.height = 5;

        auto colliders = ecs.getEntitiesWithComponent<Collider>();
        for (Entity other : colliders) {
            if (other == player) continue;

            if (!ecs.hasComponent<Transform>(other)) continue;

            auto& otherTransform = ecs.getComponent<Transform>(other);
            auto& otherCollider = ecs.getComponent<Collider>(other);

            AABB otherBox = getAABB(otherTransform, otherCollider);

            if (checkAABBCollision(groundCheckBox, otherBox)) {
                if (playerVelocity.vy >= 0) {
                    controller.isGrounded = true;
                    break;
                }
            }
        }
    }
}