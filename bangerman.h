// ============================================================================
// BangDev — BangerMan v0.1
// Tiny 2D render command recorder for pixel-art & general 2D games.
// Single-header library (C99).
//
// - Backend-agnostic: does NOT talk to SDL/OpenGL/etc.
// - Records a list of 2D draw commands in a logical coordinate space.
// - Pixel-art friendly: integer-ish API, but uses floats internally so
//   it can scale to "general 2D" without breaking the interface.
//
// Usage:
//
//   // In ONE .c file:
//   #define BANGERMAN_IMPLEMENTATION
//   #include "bangerman.h"
//
//   // In all other .c files:
//   #include "bangerman.h"
//
//   // Typical flow:
//
//   BM_Context *ctx = bm_create(1024);  // max commands per frame
//   bm_make_current(ctx);
//   bm_set_logical_size(320.0f, 180.0f);
//   bm_set_clear_color(bm_color_rgba(0.05f, 0.05f, 0.1f, 1.0f));
//
//   while (running) {
//       bm_begin_frame();
//
//       bm_set_draw_color(bm_color_rgb(1.0f, 0.0f, 0.0f));
//       bm_rect_fill(10.0f, 10.0f, 50.0f, 30.0f);
//
//       bm_set_draw_color(bm_color_rgb(0.0f, 1.0f, 0.0f));
//       bm_line(0.0f, 0.0f, 319.0f, 179.0f);
//
//       bm_end_frame();
//
//       // Backend (e.g. SDL3) consumes the recorded commands:
//       // bm_get_commands(ctx, &cmds, &count);
//       // BM_SDL3_Render(...);
//   }
//
// ============================================================================

#ifndef BANGERMAN_H
#define BANGERMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>   // bool for bm_has_overflowed

// ============================================================
// Public types
// ============================================================

// Opaque context handle
typedef struct BM_Context BM_Context;

// Color type (float 0..1). Easy to use for both pixel-art and general 2D.
typedef struct {
    float r, g, b, a;
} BM_Color;

// 2D vector type
typedef struct {
    float x, y;
} BM_Vec2;

// Abstract texture identifier
// Backends decide what this maps to (SDL_Texture*, GL texture, etc.)
typedef int BM_TextureId;
#define BM_TEXTUREID_INVALID (-1)

// Command types exposed for backends.
// Users can ignore this if they don't write a renderer.
typedef enum {
    BM_COMMAND_CLEAR = 0,
    BM_COMMAND_RECT_FILL,
    BM_COMMAND_RECT_OUTLINE,
    BM_COMMAND_LINE,
    BM_COMMAND_SPRITE
} BM_CommandType;

// Command structure exposed read-only for backends.
// Users can inspect it for debugging or custom renderers.
typedef struct {
    BM_CommandType type;
    BM_Color       color;     // draw color used for this command

    union {
        struct {              // RECT_FILL / RECT_OUTLINE
            float x, y;
            float w, h;
        } rect;

        struct {              // LINE
            float x0, y0;
            float x1, y1;
        } line;

        struct {              // SPRITE
            BM_TextureId texture;
            float dst_x, dst_y;
            float dst_w, dst_h;
            float src_x, src_y;
            float src_w, src_h;
        } sprite;
    } data;
} BM_Command;

// ============================================================
// Helper constructors
// ============================================================

// Convenience constructors for colors
static inline BM_Color bm_color_rgba(float r, float g, float b, float a) {
    BM_Color c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
}
static inline BM_Color bm_color_rgb(float r, float g, float b) {
    return bm_color_rgba(r, g, b, 1.0f);
}

// Convenience constructors for vec2
static inline BM_Vec2 bm_vec2(float x, float y) {
    BM_Vec2 v; v.x = x; v.y = y; return v;
}

// ============================================================
// Context / lifecycle
// ============================================================

// Create a BangerMan context with a fixed command capacity per frame.
// If command_capacity is <= 0, a sensible default will be used.
BM_Context* bm_create(int command_capacity);

// Destroy a context created with bm_create.
void bm_destroy(BM_Context* ctx);

// Set this context as the global "current" one.
// All bm_* functions that do not take a BM_Context* explicitly will
// operate on this context.
void bm_make_current(BM_Context* ctx);

// ============================================================
// Configuration
// ============================================================

// Set logical canvas size (e.g. 320x180 for pixel-art).
// Backends are expected to map this logical space to the actual output
// resolution (e.g. via integer scaling).
void bm_set_logical_size(float width, float height);

// Get the logical size previously set.
void bm_get_logical_size(float* out_width, float* out_height);

// Set clear color (used when emitting BM_COMMAND_CLEAR in bm_begin_frame()).
void bm_set_clear_color(BM_Color color);

// Set current draw color used by subsequent primitives.
void bm_set_draw_color(BM_Color color);

// ============================================================
// Frame control
// ============================================================

// Begin a new frame:
// - Resets the internal command buffer.
// - Records a BM_COMMAND_CLEAR using the current clear color.
void bm_begin_frame(void);

// End the frame:
// - Does not render anything by itself.
// - Backends should call bm_get_commands() after bm_end_frame() to
//   iterate over the recorded commands and draw them.
void bm_end_frame(void);

// ============================================================
// Primitives (float-based, but pixel-art friendly)
// ============================================================

// Draw a filled rectangle at (x, y) with size (w, h) in logical units.
void bm_rect_fill(float x, float y, float w, float h);

// Draw a rectangle outline at (x, y) with size (w, h) in logical units.
void bm_rect_outline(float x, float y, float w, float h);

// Draw a line from (x0, y0) to (x1, y1) in logical units.
void bm_line(float x0, float y0, float x1, float y1);

// Draw a sprite (textured quad). Texture ID is opaque and backend-specific.
// src_* is the source rectangle in texture space.
// dst_* is the destination rectangle in logical coordinates.
void bm_sprite(BM_TextureId texture,
               float dst_x, float dst_y, float dst_w, float dst_h,
               float src_x, float src_y, float src_w, float src_h);

// ============================================================
// Backend / debug access
// ============================================================

// Get a read-only pointer to the recorded commands for the last frame.
// out_commands will point to an internal array owned by the context.
// Do NOT modify it. It is valid until the next bm_begin_frame() call.
void bm_get_commands(const BM_Context* ctx,
                     const BM_Command** out_commands,
                     int* out_count);

// Get the current command capacity (max commands per frame).
int bm_get_command_capacity(const BM_Context* ctx);

// Get the number of commands recorded in the last frame.
int bm_get_command_count(const BM_Context* ctx);

// Returns true if at some point commands overflowed the capacity in the
// current frame. Overflow means extra commands were silently dropped.
// The flag is reset automatically at bm_begin_frame().
bool bm_has_overflowed(const BM_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif // BANGERMAN_H

// ============================================================================
// Implementation
// ============================================================================

#ifdef BANGERMAN_IMPLEMENTATION

#include <stdlib.h> // malloc, free

// ------------------------------------------------------------
// Internal context structure
// ------------------------------------------------------------

struct BM_Context {
    float logical_w;
    float logical_h;

    BM_Color clear_color;
    BM_Color current_color;

    BM_Command* commands;
    int         command_count;
    int         command_capacity;

    int         overflowed;   // non-zero if we dropped commands this frame
};

// Global current context (simple v0.1 approach)
static BM_Context* g_bm_ctx = NULL;

// ------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------

static int bm__ensure_capacity(BM_Context* ctx, int needed_extra) {
    if (!ctx) return 0;
    int needed = ctx->command_count + needed_extra;
    if (needed <= ctx->command_capacity) {
        return 1;
    }
    // v0.1: hard cap. Mark overflow so caller can detect/debug.
    ctx->overflowed = 1;
    return 0;
}

static BM_Command* bm__push_command(BM_Context* ctx) {
    if (!ctx) return NULL;
    if (!bm__ensure_capacity(ctx, 1)) {
        return NULL;
    }
    BM_Command* cmd = &ctx->commands[ctx->command_count++];
    return cmd;
}

// ------------------------------------------------------------
// Public API implementation
// ------------------------------------------------------------

BM_Context* bm_create(int command_capacity) {
    if (command_capacity <= 0) {
        command_capacity = 1024; // default
    }

    BM_Context* ctx = (BM_Context*)malloc(sizeof(BM_Context));
    if (!ctx) return NULL;

    ctx->logical_w = 320.0f;
    ctx->logical_h = 180.0f;

    ctx->clear_color   = bm_color_rgb(0.0f, 0.0f, 0.0f);
    ctx->current_color = bm_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

    ctx->commands = (BM_Command*)malloc(sizeof(BM_Command) * command_capacity);
    if (!ctx->commands) {
        free(ctx);
        return NULL;
    }

    ctx->command_count    = 0;
    ctx->command_capacity = command_capacity;
    ctx->overflowed       = 0;

    return ctx;
}

void bm_destroy(BM_Context* ctx) {
    if (!ctx) return;
    if (g_bm_ctx == ctx) {
        g_bm_ctx = NULL;
    }
    free(ctx->commands);
    free(ctx);
}

void bm_make_current(BM_Context* ctx) {
    g_bm_ctx = ctx;
}

void bm_set_logical_size(float width, float height) {
    if (!g_bm_ctx) return;
    if (width <= 0.0f || height <= 0.0f) return;
    g_bm_ctx->logical_w = width;
    g_bm_ctx->logical_h = height;
}

void bm_get_logical_size(float* out_width, float* out_height) {
    if (!g_bm_ctx) return;
    if (out_width)  *out_width  = g_bm_ctx->logical_w;
    if (out_height) *out_height = g_bm_ctx->logical_h;
}

void bm_set_clear_color(BM_Color color) {
    if (!g_bm_ctx) return;
    g_bm_ctx->clear_color = color;
}

void bm_set_draw_color(BM_Color color) {
    if (!g_bm_ctx) return;
    g_bm_ctx->current_color = color;
}

void bm_begin_frame(void) {
    if (!g_bm_ctx) return;
    g_bm_ctx->command_count = 0;
    g_bm_ctx->overflowed    = 0;

    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_COMMAND_CLEAR;
    cmd->color = g_bm_ctx->clear_color;
}

void bm_end_frame(void) {
    // v0.1: nothing to do here.
}

void bm_rect_fill(float x, float y, float w, float h) {
    if (!g_bm_ctx) return;

    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_COMMAND_RECT_FILL;
    cmd->color = g_bm_ctx->current_color;
    cmd->data.rect.x = x;
    cmd->data.rect.y = y;
    cmd->data.rect.w = w;
    cmd->data.rect.h = h;
}

void bm_rect_outline(float x, float y, float w, float h) {
    if (!g_bm_ctx) return;

    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_COMMAND_RECT_OUTLINE;
    cmd->color = g_bm_ctx->current_color;
    cmd->data.rect.x = x;
    cmd->data.rect.y = y;
    cmd->data.rect.w = w;
    cmd->data.rect.h = h;
}

void bm_line(float x0, float y0, float x1, float y1) {
    if (!g_bm_ctx) return;

    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_COMMAND_LINE;
    cmd->color = g_bm_ctx->current_color;
    cmd->data.line.x0 = x0;
    cmd->data.line.y0 = y0;
    cmd->data.line.x1 = x1;
    cmd->data.line.y1 = y1;
}

void bm_sprite(BM_TextureId texture,
               float dst_x, float dst_y, float dst_w, float dst_h,
               float src_x, float src_y, float src_w, float src_h) {
    if (!g_bm_ctx) return;

    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_COMMAND_SPRITE;
    cmd->color = g_bm_ctx->current_color; // tint / modulate color

    cmd->data.sprite.texture = texture;
    cmd->data.sprite.dst_x   = dst_x;
    cmd->data.sprite.dst_y   = dst_y;
    cmd->data.sprite.dst_w   = dst_w;
    cmd->data.sprite.dst_h   = dst_h;
    cmd->data.sprite.src_x   = src_x;
    cmd->data.sprite.src_y   = src_y;
    cmd->data.sprite.src_w   = src_w;
    cmd->data.sprite.src_h   = src_h;
}

void bm_get_commands(const BM_Context* ctx,
                     const BM_Command** out_commands,
                     int* out_count) {
    if (!ctx) return;
    if (out_commands) *out_commands = ctx->commands;
    if (out_count)    *out_count    = ctx->command_count;
}

int bm_get_command_capacity(const BM_Context* ctx) {
    if (!ctx) return 0;
    return ctx->command_capacity;
}

int bm_get_command_count(const BM_Context* ctx) {
    if (!ctx) return 0;
    return ctx->command_count;
}

bool bm_has_overflowed(const BM_Context* ctx) {
    if (!ctx) return false;
    return ctx->overflowed != 0;
}

#endif // BANGERMAN_IMPLEMENTATION
