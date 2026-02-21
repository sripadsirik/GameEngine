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