#pragma once

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