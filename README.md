# Custom 2D Game Engine

A 2D game engine built from scratch in C++ to learn game engine architecture, graphics programming, and systems design.

## 🎯 Project Goals

- **Deep Understanding**: Build everything from scratch to truly understand how game engines work
- **Systems Programming**: Master memory management, performance optimization, and data-oriented design
- **Graphics Programming**: Learn rendering pipelines, batching, and texture management
- **Portfolio Piece**: Create a well-architected, documented engine with a playable demo game

## 🚀 Current Status

**Phase 1: Foundation** ✅ (Completed)
- [x] SDL2 integration
- [x] Window creation and management
- [x] Basic game loop (60 FPS)
- [x] Event handling (keyboard, window events)
- [x] Basic rendering (shapes, colors)

**Upcoming Phases:**
- Phase 2: Rendering System (texture loading, sprites, camera)
- Phase 3: Entity Component System (custom ECS architecture)
- Phase 4: Physics & Collision (AABB, spatial partitioning)
- Phase 5: Game Features (animations, particles, audio, UI)
- Phase 6: Tools & Polish (scene serialization, debug tools)

## 🛠️ Tech Stack

- **Language**: C++17
- **Graphics Library**: SDL2
- **Build System**: CMake
- **Platform**: Windows (cross-platform planned)

## 📋 Prerequisites

- Visual Studio 2022 (or Build Tools with Desktop C++ development)
- CMake 3.20+
- SDL2 2.30.10 (instructions below)

## 🔧 Setup Instructions

### 1. Install SDL2

1. Download [SDL2-devel-2.30.10-VC.zip](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.10)
2. Extract to `C:\SDL2`
3. Verify structure:
```
   C:\SDL2\
   ├── include\
   ├── lib\x64\
   └── docs\
```

### 2. Clone and Build
```bash
git clone https://github.com/YOUR_USERNAME/GameEngine.git
cd GameEngine
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

### 3. Run
```bash
# From build directory
Debug\GameEngine.exe
```

Or use VS Code:
- Press `Ctrl+Shift+P` → "CMake: Build"
- Press `F5` to run

## 🎮 Controls

- **ESC** - Exit application
- **X button** - Close window

## 📁 Project Structure
```
GameEngine/
├── src/              # Source files
│   └── main.cpp      # Entry point & game loop
├── include/          # Header files
├── assets/           # Game assets (textures, sounds)
├── build/            # Build output (gitignored)
├── CMakeLists.txt    # Build configuration
└── README.md
```

## 🧠 What I'm Learning

- **Game Loop Architecture**: Fixed timestep updates, variable rendering, FPS control
- **Event-Driven Programming**: Handling user input and window events
- **Graphics Fundamentals**: Renderers, double buffering, draw calls
- **Build Systems**: CMake configuration, library linking
- **Memory Management**: Resource cleanup, preventing leaks

## 📚 Development Timeline

- **Week 1-2**: Foundation (window, input, game loop) ✅
- **Week 3-4**: Rendering system (textures, sprites, camera)
- **Week 5-6**: Entity Component System
- **Week 7-8**: Physics & collision
- **Week 9-10**: Game features
- **Week 11-12**: Polish & demo game

**Estimated completion**: ~12-24 weeks (5 hours/week)

## 🎯 Target Demo Game

Planning to build a simple platformer or top-down shooter to demonstrate engine capabilities:
- Player movement and physics
- Enemies with AI
- Collectibles and scoring
- Multiple levels
- Sound effects and music

## 📖 Resources

- [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- [LazyFoo SDL2 Tutorials](https://lazyfoo.net/tutorials/SDL/)
- [LearnOpenGL](https://learnopengl.com/)
- [Game Engine Architecture (Book)](https://www.gameenginebook.com/)

## 📝 License

This project is for educational purposes.

## 🤝 Contributing

This is a personal learning project, but feedback and suggestions are welcome!

---

**Built with ❤️ as a learning journey into game engine development**