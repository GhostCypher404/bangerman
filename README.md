# BangerMan — Tiny 2D Render Command Recorder (Single-Header Library) v.02

**BangerMan** is a lightweight, backend-agnostic **2D render command recorder** written in C (C99).  
It is designed for **pixel-art engines**, **retro toolchains**, and **general 2D games**, following the philosophy of libraries like **stb**, **nanovg**, and **clay**.

BangerMan does **not** render anything by itself.  
Instead, it records drawing commands (rects, lines, sprites…) into a compact buffer, which any backend can consume (e.g., SDL3, Sokol, OpenGL2, software rasterizer).

This makes it ideal for:
- custom engines
- pixel-art editors
- retro-inspired games
- educational projects
- tools that need a simple, predictable renderer interface

---

## ✨ Features

- **Single-header** (`bangerman.h`)
- **Zero dependencies** (backend agnostic)
- **Pixel-art friendly** (integer scaling pipeline)
- **Float-based API** (safe for general 2D too)
- **Command-buffer architecture** (like modern engines)
- **Backends live in `/backends`** (not required by the core)
- **Simple integration** (drop into any C project)
- **Extremely small** and easy to read

---

## 📦 Repository Structure

bangerman/
bangerman.h                 # core (single header)
backends/
SDL3/
bm_renderer_SDL3.c      # SDL3 backend (optional)
examples/
SDL3-basic/
main.c                  # minimal demo using SDL3
README.md
LICENSE

---

## 🚀 Getting Started

### 1. Add BangerMan to your project

```c
#define BANGERMAN_IMPLEMENTATION
#include "bangerman.h"

2. (Optional) Include an SDL3 backend

#include "backends/SDL3/bm_renderer_SDL3.c"

3. Basic usage

BM_Context *bm = bm_create(1024);
bm_make_current(bm);
bm_set_logical_size(320.0f, 180.0f);

while (running) {
    bm_begin_frame();

    bm_set_draw_color(bm_color_rgb(1.0f, 0.0f, 0.0f));
    bm_rect_fill(10, 10, 50, 30);

    bm_set_draw_color(bm_color_rgb(0.0f, 1.0f, 0.0f));
    bm_line(0, 0, 319, 179);

    bm_end_frame();

    // Render using any backend
    // Example: BM_SDL3_Render(&renderer, bm);
}


⸻

🧩 Backends

BangerMan ships with a reference backend:
	•	SDL3 — backends/SDL3/bm_renderer_SDL3.c

Future community backends may include:
	•	OpenGL2/Legacy GL
	•	Sokol-gfx
	•	Software rasterizer
	•	Metal / Vulkan (via thin layers)
	•	Web (WASM + HTML Canvas)

Backends are intentionally simple and easy to understand.

⸻

🛠 Design Philosophy

BangerMan follows three core principles:

1. Predictability over abstraction

The API is tiny and transparent: users understand exactly what is happening.

2. Compatibility over specialization

Works in game engines, tools, editors, or embedded SDL apps.

3. Retro-first, but future-ready

Pixel-art? Perfect.
Need subpixel accuracy later? Already supported.

⸻

📚 Example Project

A minimal SDL3 example lives at:

examples/SDL3-basic/main.c

Compile with:

cc main.c -I../../ -lSDL3 -o banger_example


⸻

📝 License

MIT License — free for commercial and non-commercial use.

⸻

🤝 Contributing

PRs, issues, and backends are welcome!

If you create a backend (GL, Sokol, raylib, software…), feel free to submit it.

⸻

💬 Author

Developed as part of the BangDev ecosystem —
low-level, handcrafted tools for retro-inspired engines and digital art.

