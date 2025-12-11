// // ============================================================
// BangerMan — Tiny 2D Render Command Recorder (Single-Header)
// BangDev / CRAYON playground edition
// ------------------------------------------------------------
// - Backend agnostic (records commands only)
// - Pixel-art friendly (logical canvas)
// - Single-header style API
// ============================================================
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

#include <stdint.h>

// ------------------------------------------------------------
// Public types
// ------------------------------------------------------------

typedef struct {
    float r, g, b, a;
} BM_Color;

typedef int32_t BM_TextureId;

// Opaque context handle
typedef struct BM_Context BM_Context;

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------

// Context lifecycle
BM_Context* bm_create(int command_capacity);
void        bm_destroy(BM_Context* ctx);
void        bm_make_current(BM_Context* ctx);

// Logical canvas size
void bm_set_logical_size(float width, float height);
void bm_get_logical_size(float* out_width, float* out_height);

// Clear / draw color
void     bm_set_clear_color(BM_Color color);
BM_Color bm_get_clear_color(void);  // <- NEW: needed by SDL3 backend
void     bm_set_draw_color(BM_Color color);

// Frame boundary
void bm_begin_frame(void);
void bm_end_frame(void);

// Basic primitives
void bm_rect_fill(float x, float y, float w, float h);
void bm_rect_outline(float x, float y, float w, float h);
void bm_line(float x0, float y0, float x1, float y1);

// Sprites
void bm_sprite(BM_TextureId texture,
               float x, float y,
               float w, float h);

// Command buffer readback
typedef enum {
    BM_CMD_RECT_FILL = 1,
    BM_CMD_RECT_OUTLINE,
    BM_CMD_LINE,
    BM_CMD_SPRITE,
} BM_CommandType;

typedef struct {
    BM_CommandType type;
    BM_Color       color;
    float          x, y, w, h;
    float          x2, y2;      // For lines
    BM_TextureId   texture;     // For sprites
} BM_Command;

typedef struct {
    BM_Command* commands;
    int         count;
} BM_CommandView;

void bm_get_commands(const BM_Context* ctx,
                     BM_CommandView*   out_view);

// Color helpers
static inline BM_Color bm_color_rgba(float r, float g, float b, float a) {
    BM_Color c = { r, g, b, a };
    return c;
}

static inline BM_Color bm_color_rgb(float r, float g, float b) {
    return bm_color_rgba(r, g, b, 1.0f);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BANGERMAN_H

// ============================================================
// IMPLEMENTATION
// ============================================================

#ifdef BANGERMAN_IMPLEMENTATION
#ifndef BANGERMAN_IMPLEMENTATION_DONE
#define BANGERMAN_IMPLEMENTATION_DONE

#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ------------------------------------------------------------
// Internal types
// ------------------------------------------------------------

struct BM_Context {
    BM_Command* commands;
    int         capacity;
    int         count;

    float logical_width;
    float logical_height;

    BM_Color clear_color;
    BM_Color draw_color;
};

// Global current context pointer
static BM_Context* g_bm_ctx = NULL;

// ------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------

static int
bm__ensure_capacity(BM_Context* ctx, int needed_extra)
{
    if (!ctx) return 0;
    int required = ctx->count + needed_extra;
    if (required <= ctx->capacity) return 1;

    int new_cap = ctx->capacity ? ctx->capacity * 2 : 64;
    if (new_cap < required) {
        new_cap = required;
    }

    BM_Command* new_buf =
        (BM_Command*)realloc(ctx->commands, (size_t)new_cap * sizeof(BM_Command));
    if (!new_buf) return 0;

    ctx->commands = new_buf;
    ctx->capacity = new_cap;
    return 1;
}

static BM_Command*
bm__push_command(BM_Context* ctx)
{
    if (!bm__ensure_capacity(ctx, 1)) {
        return NULL;
    }
    BM_Command* cmd = &ctx->commands[ctx->count++];
    memset(cmd, 0, sizeof(*cmd));
    return cmd;
}

// ------------------------------------------------------------
// Public API implementation
// ------------------------------------------------------------

BM_Context*
bm_create(int command_capacity)
{
    if (command_capacity <= 0) {
        command_capacity = 64;
    }

    BM_Context* ctx = (BM_Context*)calloc(1, sizeof(BM_Context));
    if (!ctx) return NULL;

    ctx->commands = (BM_Command*)calloc((size_t)command_capacity,
                                        sizeof(BM_Command));
    if (!ctx->commands) {
        free(ctx);
        return NULL;
    }

    ctx->capacity       = command_capacity;
    ctx->count          = 0;
    ctx->logical_width  = 320.0f;
    ctx->logical_height = 180.0f;
    ctx->clear_color    = bm_color_rgba(0.0f, 0.0f, 0.0f, 1.0f);
    ctx->draw_color     = bm_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);

    return ctx;
}

void
bm_destroy(BM_Context* ctx)
{
    if (!ctx) return;
    if (g_bm_ctx == ctx) {
        g_bm_ctx = NULL;
    }
    free(ctx->commands);
    free(ctx);
}

void
bm_make_current(BM_Context* ctx)
{
    g_bm_ctx = ctx;
}

void
bm_set_logical_size(float width, float height)
{
    if (!g_bm_ctx) return;
    g_bm_ctx->logical_width  = width;
    g_bm_ctx->logical_height = height;
}

void
bm_get_logical_size(float* out_width, float* out_height)
{
    if (!g_bm_ctx) return;
    if (out_width)  *out_width  = g_bm_ctx->logical_width;
    if (out_height) *out_height = g_bm_ctx->logical_height;
}

void
bm_set_clear_color(BM_Color color)
{
    if (!g_bm_ctx) return;
    g_bm_ctx->clear_color = color;
}

BM_Color
bm_get_clear_color(void)
{
    if (!g_bm_ctx) {
        BM_Color c = {0.0f, 0.0f, 0.0f, 1.0f};
        return c;
    }
    return g_bm_ctx->clear_color;
}

void
bm_set_draw_color(BM_Color color)
{
    if (!g_bm_ctx) return;
    g_bm_ctx->draw_color = color;
}

void
bm_begin_frame(void)
{
    if (!g_bm_ctx) return;
    g_bm_ctx->count = 0;
    // Clear is logical only; backends decide how to use clear_color.
}

void
bm_end_frame(void)
{
    // Nothing special for now; backends will read commands afterwards.
}

void
bm_rect_fill(float x, float y, float w, float h)
{
    if (!g_bm_ctx) return;
    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_CMD_RECT_FILL;
    cmd->color = g_bm_ctx->draw_color;
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
}

void
bm_rect_outline(float x, float y, float w, float h)
{
    if (!g_bm_ctx) return;
    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_CMD_RECT_OUTLINE;
    cmd->color = g_bm_ctx->draw_color;
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
}

void
bm_line(float x0, float y0, float x1, float y1)
{
    if (!g_bm_ctx) return;
    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type  = BM_CMD_LINE;
    cmd->color = g_bm_ctx->draw_color;
    cmd->x  = x0;
    cmd->y  = y0;
    cmd->x2 = x1;
    cmd->y2 = y1;
}

void
bm_sprite(BM_TextureId texture,
          float x, float y,
          float w, float h)
{
    if (!g_bm_ctx) return;
    BM_Command* cmd = bm__push_command(g_bm_ctx);
    if (!cmd) return;

    cmd->type    = BM_CMD_SPRITE;
    cmd->color   = g_bm_ctx->draw_color;
    cmd->texture = texture;
    cmd->x       = x;
    cmd->y       = y;
    cmd->w       = w;
    cmd->h       = h;
}

void
bm_get_commands(const BM_Context* ctx,
                BM_CommandView*   out_view)
{
    if (!ctx || !out_view) return;
    out_view->commands = ctx->commands;
    out_view->count    = ctx->count;
}

#endif // BANGERMAN_IMPLEMENTATION_DONE
#endif // BANGERMAN_IMPLEMENTATION
