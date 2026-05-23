# Block Breaker - HARD

## 💻 Overview
A high-performance **Block Breaker** game developed in **C++** using **SDL3** and **GLM**. This project features a sophisticated collision engine, multi-stage brick durability logic, and real-time color interpolation for visual feedback. It pushes the boundaries of standard arcade mechanics by implementing a dynamic "hit-animation" system and custom circle rasterization.

---

## 🤍 Tech Stack
* **Language:** C++
* **Graphics & Input:** SDL3
* **Mathematics:** GLM (Linear Algebra)
* **Core Concepts:** Game Loop, Delta-Time Physics, Collision Detection, Linear Interpolation (Lerp)

![C++](https://img.shields.io/badge/C%2B%2B-Language-%23FF69B4.svg?style=for-the-badge&logo=cplusplus&logoColor=white) ![SDL3](https://img.shields.io/badge/SDL3-Game_Dev-%23FF69B4.svg?style=for-the-badge&logo=sdl&logoColor=white)

---

## 🎀 Core Functionality
* **Advanced Physics Engine:** Precise collision detection between the ball (Circle) and game objects (Rectangles), featuring angle-based rebound calculations depending on the point of impact on the paddle.
* **Brick Durability System:** Multi-life brick mechanics where blocks transition through different states of integrity before destruction.
* **Dynamic Visual Feedback:** Features a **color-interpolation (lerp) animation system** that gradually shifts brick colors over a 2-second period after each hit.
* **Game States:** Implements robust state management (PLAYING, WON, LOST) with visual signaling (e.g., paddle color shifting to red on loss or green on victory).

---

## 📂 Project Structure
* **`main.cpp`**: Manages the game loop, initialization, and core physics/animation logic.
* **`circle.h / circle.cpp`**: Custom implementation of circle primitives and manual rasterization algorithms.
* **`Rectangle Struct`**: Manages entity properties including health (lives), hit animations, and rendering logic.

---

© 2025 Glitter Geeks Coderun | Developed by [**𝐅𝐥𝐨𝐫𝐞𝐚𝐧 𝐄𝐦𝐢𝐥𝐢𝐚-𝐀𝐥𝐞𝐱𝐚𝐧𝐝𝐫𝐚**](https://github.com/Emily-f2510), [**𝐋𝐞𝐨𝐧𝐭𝐞 𝐏𝐚𝐭𝐫𝐢𝐜𝐢𝐚-𝐌𝐢𝐫𝐚𝐛𝐞𝐥𝐚**](https://patrrrrrrricia.github.io/glowing-button/), [**𝐋𝐮𝐩𝐚𝐧𝐜𝐮 𝐆𝐚𝐛𝐫𝐢𝐞𝐥𝐚-𝐕𝐚𝐥𝐞𝐧𝐭𝐢𝐧𝐚**](https://github.com/gabrielalupancu)
