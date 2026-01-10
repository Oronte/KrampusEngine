![KrampusEngine](Content/Textures/KrampusEngineBanner.png)

# KrampusEngine

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![Build](https://img.shields.io/badge/Build-Visual%20Studio-blueviolet)
![Library](https://img.shields.io/badge/Engine-Static%20Library-important)
![Status](https://img.shields.io/badge/Status-Active-success)

---

## Overview

**KrampusEngine** is an experimental **2D game engine** written in **modern C++20**, built on top of **SFML 3.0.0**.  
It is a **code-driven engine** with a strict separation between engine runtime systems and game logic.

This project is designed as a **learning-focused but production-minded** engine: prioritizing clarity, correctness, and architecture over features or tooling.

The engine is compiled as a **static library**, while games are built as separate executables linked against it.

---

## Design Goals

- Clean and maintainable C++20 code
- Clear separation of responsibilities
- RAII-first architecture with deterministic lifetime management
- Explicit control flow over hidden magic
- Practical engine architecture suitable for technical evaluation

This project intentionally avoids editor tooling and scripting layers to keep the focus on **engine internals**.

---

## Technology Stack

| Area | Details |
|-----|--------|
| Language | C++20 |
| Platform | Windows |
| Rendering / Windowing | SFML 3.0.0 |
| Build System | Premake5 |
| IDE | Visual Studio |
| Engine Output | Static Library (`.lib`) |
| Game Output | Executable (`.exe`) |

---

## Project Structure

### Engine
- Contains all runtime systems
- No game-specific logic
- Owns memory, timing, and execution flow

### Game
- Depends on the engine
- Defines actors, components, and levels
- Contains all gameplay rules and content

---

## Architecture

### Actor / Component Model

KrampusEngine uses a **hybrid Actor–Component architecture**:

- **Actors**
  - Own high-level game logic
  - Control lifecycle and behavior
- **Components**
  - Modular, reusable functionality
  - Unity-style composition
  - Attached to actors

This hybrid approach allows expressive gameplay code without sacrificing structural clarity.

---

### Level System

- Levels are defined as **C++ classes**
- Initialized explicitly at runtime
- No data-driven or serialized formats (by design)

This keeps level flow explicit and easy to trace during debugging and reviews.

---

## Core Systems

### Memory Management

- Fully automatic memory management
- **Smart pointers only**
- No raw ownership
- No direct dynamic allocation by the user
- RAII-based lifetime control
- Memory leak detection enabled in Debug builds using Windows diagnostics

---

### Event System

- Synchronous observer / listener model
- Explicit event dispatch
- No hidden async behavior
- Designed for predictability and ease of reasoning

---

### Input System

- Centralized input handling
- Abstracted from SFML events
- Accessible from gameplay code

---

### Collision System (2D)

Custom-built collision system supporting:

- AABB (Axis-Aligned Bounding Box)
- OBB (Oriented Bounding Box)
- Circle–Circle
- Circle–Rectangle

Focused on correctness and clarity rather than physics simulation.

---

### Animation System

- Sprite-based animation
- Spritesheets
- Timer-driven frame control
- Deterministic playback

---

### Logging System

- Dedicated logging thread
- Thread-isolated logging pipeline
- Colored console output
- Persistent file output (`log.txt`)
- Disabled entirely in Release builds

---

## Debug and Build Modes

### Debug Build
- Logging enabled
- Debug output enabled
- Memory leak detection enabled
- Console window enabled

### Release Build
- Logging disabled
- No debug output
- Optimized build
- No console window

---

## Limitations

- Windows-only
- 2D only
- No editor or GUI
- Engine is **not thread-safe**  
  *(intentional design choice to reduce complexity and improve determinism)*

These constraints are deliberate and aligned with the project’s learning goals.

---

## Project Status

- Actively developed
- Core engine systems implemented
- Demo game currently in progress
- Architecture considered stable, systems evolving

---

## Motivation

KrampusEngine exists as a **personal engineering project** with the following goals:

- Learn and apply engine architecture principles
- Improve modern C++ design and ownership models
- Build a complete engine rather than relying on existing ones
- Create a codebase suitable for technical discussion and review

This project prioritizes **understanding and correctness** over speed of iteration or feature breadth.

---

## License

This project is intended for educational and personal use.  
License information will be added once the engine reaches a stable milestone.
