#pragma once
#include <vector>
#include <string>

struct Transform {
    float x;
    float y;
    float rotation;
    float scaleX;
    float scaleY;
};

struct Sprite {
    void* texture;
    int width;
    int height;
    int srcX;
    int srcY;
    int srcWidth;
    int srcHeight;
};

struct Velocity {
    float vx;
    float vy;
};

struct Collider {
    float width;
    float height;
    float offsetX;
    float offsetY;
    bool isTrigger;
};

struct RigidBody {
    float mass;
    bool useGravity;
    float gravityScale;
    bool isStatic;
};

struct PlayerController {
    float speed;
    float jumpForce;
    bool isGrounded;
};

struct Animation {
    void* spriteSheet;
    int frameWidth;
    int frameHeight;
    int totalFrames;
    int currentFrame;
    float frameTime;
    float elapsedTime;
    bool loop;
    bool playing;
};

struct ParticleEmitter {
    float emissionRate;
    float particleLifetime;
    float timeSinceLastEmit;
    int maxParticles;
    bool active;
    float minVelocityX;
    float maxVelocityX;
    float minVelocityY;
    float maxVelocityY;
};

struct Particle {
    float lifetime;
    float age;
    int colorR;
    int colorG;
    int colorB;
    int colorA;
};

struct Health {
    int current;
    int max;
};

struct Lifetime {
    float duration;
    float elapsed;
};