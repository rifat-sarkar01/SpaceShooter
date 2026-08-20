<div align="center">

# `> SPACE SHOOTER_`

### _Rage-Fang Edition_

<br/>

A retro-inspired 2D space shooter built from scratch in **C++** & **OpenGL**.
No engine. No frameworks. Just raw triangles and passion.

<br/>

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green?logo=opengl&logoColor=white)](https://www.khronos.org/opengl/)
[![GLFW](https://img.shields.io/badge/GLFW-3.x-red)](https://www.glfw.org/)
[![GLEW](https://img.shields.io/badge/GLEW-2.x-orange)](http://glew.sourceforge.net/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](#license)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)]()

</div>

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🎨 **Toon-Shaded Ship** | Hand-crafted "Rage-Fang" starship with 20+ polygonal parts, black outlines, and engine glow effects |
| 👾 **Alien Enemies** | Terracotta-shelled aliens with swept fins, glowing cyan eyes, and procedural carapace plates |
| 🔥 **Boss Battles** | Massive boss aliens every 5 kills — with claw appendages, thruster ports, HP bars, and aimed projectiles |
| 🌌 **Starfield Background** | Procedurally generated parallax starfield with twinkling stars |
| 📊 **7-Segment HUD** | Retro segment-font score and lives display rendered via legacy OpenGL |
| 💥 **Particle Effects** | Muzzle flash, engine exhaust plumes, and explosion highlights |
| 🎯 **Precision Aiming** | Mouse-driven ship rotation with WASD movement |
| ⚡ **Performance** | Efficient modern GL pipeline for ship rendering + legacy GL for HUD and effects |
| 🔁 **Infinite Replay** | Endless wave system with accelerating spawn rates as score increases |

---

## 🎮 Gameplay

```
┌─────────────────────────────────────────────┐
│  WASD ............. Move the ship            │
│  MOUSE ............ Aim                      │
│  LEFT CLICK ........ Fire                    │
│  R ................. Restart (after death)    │
│                                             │
│  ● Destroy normal aliens for 10 points       │
│  ● Every 5 kills spawns a BOSS (+50 pts)    │
│  ● Survive as long as possible!              │
└─────────────────────────────────────────────┘
```

---

## 🛠️ Tech Stack

```
┌──────────────────────────────────────────────────────────┐
│                    TECHNOLOGY STACK                       │
├──────────────────────────────────────────────────────────┤
│                                                          │
│   Language ........... C++17                              │
│   Graphics API ....... OpenGL 3.3 (Compatibility)        │
│   Window/Input ....... GLFW 3.x                          │
│   Extension Loader ... GLEW 2.x                          │
│   Image Loading ...... stb_image.h                       │
│   Build System ....... GNU Make                           │
│   Compiler ........... GCC / MinGW-w64                   │
│                                                          │
│   Architecture:                                           │
│   ┌────────────┐  ┌────────────┐  ┌────────────┐        │
│   │  Renderer  │──│   Game     │──│  Entities  │        │
│   │ (RageFang) │  │ (Loop/IO)  │  │ (Aliens,   │        │
│   │            │  │            │  │  Player,   │        │
│   │  Modern GL │  │  Collision │  │  Project.) │        │
│   └────────────┘  └────────────┘  └────────────┘        │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

---

## 📁 Project Structure

```
SpaceShooter/
├── main.cpp              # Entry point
├── game.h / game.cpp     # Game loop, state management, GLFW callbacks
├── player.h / player.cpp # Player ship logic (movement, aiming, firing)
├── alien.h / alien.cpp   # Alien & boss behavior, procedural rendering
├── projectile.h / .cpp   # Bullet physics and lifecycle
├── renderer.h / renderer.cpp  # RageFangRenderer — toon ship with modern GL
├── common.h / common.cpp # Shared types (Vec2), math helpers, constants
├── stb_image.h           # Single-header image loader
├── Space_shooter.cpp     # Monolithic version (all-in-one)
├── Makefile              # Build system (Windows + Linux)
├── ship.png              # Player ship texture
├── normal_alien.png      # Normal alien sprite
└── boss_alien.png        # Boss alien sprite
```

---

## 🚀 Getting Started

### Prerequisites

| Requirement | Windows (MSYS2) | Linux |
|-------------|-----------------|-------|
| **Compiler** | MinGW-w64 GCC | GCC / Clang |
| **GLEW** | `pacman -S mingw-w64-x86_64-glew` | `sudo apt install libglew-dev` |
| **GLFW** | `pacman -S mingw-w64-x86_64-glfw` | `sudo apt install libglfw3-dev` |
| **OpenGL** | Built-in | `sudo apt install libgl-dev` |
| **stb_image** | `pacman -S mingw-w64-x86_64-stb` | Place `stb_image.h` in project root |

### Build & Run

**Windows (MSYS2 MINGW64):**
```bash
# Clone the repo
git clone https://github.com/rifat-sarkar01/SpaceShooter.git
cd SpaceShooter

# Build
make

# Copy required DLLs (first time only)
cp /c/msys64/mingw64/bin/glew32.dll .
cp /c/msys64/mingw64/bin/glfw3.dll .

# Copy asset PNGs next to the .exe, then run
./space_shooter.exe
```

**Linux:**
```bash
# Clone the repo
git clone https://github.com/rifat-sarkar01/SpaceShooter.git
cd SpaceShooter

# Build
make PLATFORM=linux

# Run
./space_shooter
```

---

## 🏗️ Architecture Highlights

### Dual Rendering Pipeline
The game uses a **hybrid rendering approach**:
- **Modern OpenGL 3.3** — VAO/VBO-based toon shader for the player ship (with outline pass + flat fill pass)
- **Legacy OpenGL** — Immediate mode for aliens, projectiles, HUD, starfield, and particle effects

### Rage-Fang Ship Renderer
The player ship is a hand-designed polygonal model with **23 distinct parts**:
```
Fuselage, Left/Right Wings, Leading Edges, Wing Tip Fins,
Wing Detail Panels, Cockpit Canopy, Cockpit Interior,
Nose Accent, Cannon Pylons, 4 Cannon Barrels,
4 Muzzle Rings, Center Body Panel, 2 Decal Plates
```
Each part is triangulated, batched into a single VBO, and rendered with a custom vertex/fragment shader pair.

### Collision System
Simple **circle-circle collision** detection keeps the gameplay fast and responsive while maintaining clean separation between entity types.

---

## 📸 Screenshots

> *Screenshots coming soon! Run the game to see the Rage-Fang in action.*

<!-- Add screenshots here:
![Gameplay](screenshots/gameplay.png)
![Boss Battle](screenshots/boss_battle.png)
-->

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

---

<div align="center">

**Built with ❤️ and raw OpenGL**

*No game engine. No frameworks. Just C++ and triangles.*

</div>
