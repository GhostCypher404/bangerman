// ============================================================================
// BangDev — BangerMan SDL3 Renderer v0.1
// Drop-in SDL3 backend for bangerman.h
//
// Usage (example):
//
//   #define BANGERMAN_IMPLEMENTATION
//   #include "bangerman.h"
//   #include "backends/SDL3/bm_renderer_SDL3.c"
//
//   // init:
//   BM_Context *bm = bm_create(1024);
//   bm_make_current(bm);
//   bm_set_logical_size(320.0f, 180.0f);
//
//   BM_SDL3Renderer bmRenderer = {0};
//   bmRenderer.renderer = sdl_renderer;
//
//   // frame:
//   bm_begin_frame();
//   // ... issue bm_rect_fill(), bm_line(), bm_sprite() ...
//   bm_end_frame();
//
//   BM_SDL3_Render(&bmRenderer, bm);
//   SDL_RenderPresent(sdl_renderer);
// ============================================================================

#ifndef BM_SDL3_RENDERER_C
#define BM_SDL3_RENDERER_C

#include <SDL3/SDL.h>
#include "../../bangerman.h"   // ajusta o caminho conforme o layout do repo

typedef struct BM_SDL3Renderer {
    SDL_Renderer *renderer;
} BM_SDL3Renderer;

// Convert BM_Color (0..1 floats) → SDL_Uint8 RGBA
static void BM__ColorToSDL(const BM_Color *c, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
    float rf = c->r; if (rf < 0.0f) rf = 0.0f; if (rf > 1.0f) rf = 1.0f;
    float gf = c->g; if (gf < 0.0f) gf = 0.0f; if (gf > 1.0f) gf = 1.0f;
    float bf = c->b; if (bf < 0.0f) bf = 0.0f; if (bf > 1.0f) bf = 1.0f;
    float af = c->a; if (af < 0.0f) af = 0.0f; if (af > 1.0f) af = 1.0f;

    *r = (Uint8)(rf * 255.0f + 0.5f);
    *g = (Uint8)(gf * 255.0f + 0.5f);
    *b = (Uint8)(bf * 255.0f + 0.5f);
    *a = (Uint8)(af * 255.0f + 0.5f);
}

// Main render entrypoint: consume BM_Command stream and draw via SDL3.
static void BM_SDL3_Render(BM_SDL3Renderer *state, BM_Context *ctx) {
    if (!state || !state->renderer || !ctx) return;

    SDL_Renderer *r = state->renderer;

    // 1) Output size (physical pixels)
    int out_w = 0, out_h = 0;
    if (!SDL_GetRenderOutputSize(r, &out_w, &out_h)) {
        return;
    }

    // 2) Logical size from BangerMan
    float logical_w = 0.0f, logical_h = 0.0f;
    bm_get_logical_size(&logical_w, &logical_h);
    if (logical_w <= 0.0f || logical_h <= 0.0f) {
        logical_w = (float)out_w;
        logical_h = (float)out_h;
    }

    int lw = (int)(logical_w + 0.5f);
    int lh = (int)(logical_h + 0.5f);

    if (lw <= 0) lw = out_w;
    if (lh <= 0) lh = out_h;

    // 3) Integer scale (pixel-art friendly)
    int scale_x = out_w / lw;
    int scale_y = out_h / lh;
    int scale   = scale_x < scale_y ? scale_x : scale_y;
    if (scale < 1) scale = 1;

    int vp_w = lw * scale;
    int vp_h = lh * scale;
    int vp_x = (out_w - vp_w) / 2;
    int vp_y = (out_h - vp_h) / 2;

    SDL_Rect viewport;
    viewport.x = vp_x;
    viewport.y = vp_y;
    viewport.w = vp_w;
    viewport.h = vp_h;

    // 4) Fullscreen clear (black bars + background)
    SDL_SetRenderViewport(r, NULL);
    SDL_SetRenderScale(r, 1.0f, 1.0f);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    // 5) Apply logical viewport + scale
    SDL_SetRenderViewport(r, &viewport);
    SDL_SetRenderScale(r, (float)scale, (float)scale);

    // 6) Fetch commands
    const BM_Command *cmds = NULL;
    int count = 0;
    bm_get_commands(ctx, &cmds, &count);
    if (!cmds || count <= 0) {
        return;
    }

    // 7) Replay command stream
    for (int i = 0; i < count; ++i) {
        const BM_Command *cmd = &cmds[i];

        Uint8 cr, cg, cb, ca;
        BM__ColorToSDL(&cmd->color, &cr, &cg, &cb, &ca);
        SDL_SetRenderDrawColor(r, cr, cg, cb, ca);

        switch (cmd->type) {
            case BM_COMMAND_CLEAR: {
                SDL_RenderClear(r);
            } break;

            case BM_COMMAND_RECT_FILL: {
                SDL_FRect rect;
                rect.x = cmd->data.rect.x;
                rect.y = cmd->data.rect.y;
                rect.w = cmd->data.rect.w;
                rect.h = cmd->data.rect.h;
                SDL_RenderFillRect(r, &rect);
            } break;

            case BM_COMMAND_RECT_OUTLINE: {
                SDL_FRect rect;
                rect.x = cmd->data.rect.x;
                rect.y = cmd->data.rect.y;
                rect.w = cmd->data.rect.w;
                rect.h = cmd->data.rect.h;
                SDL_RenderRect(r, &rect);
            } break;

            case BM_COMMAND_LINE: {
                SDL_RenderLine(r,
                               cmd->data.line.x0,
                               cmd->data.line.y0,
                               cmd->data.line.x1,
                               cmd->data.line.y1);
            } break;

            case BM_COMMAND_SPRITE: {
                // v0.1 placeholder.
                // In a future version, you can:
                // - keep a table BM_TextureId -> SDL_Texture*
                // - look up cmd->data.sprite.texture
                // - call SDL_RenderTexture()
            } break;

            default:
                break;
        }
    }
}

#endif // BM_SDL3_RENDERER_C
