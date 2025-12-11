// ============================================================
// BM_SDL3_Render — SDL3 backend for BangerMan
// BangDev / Crayon playground
// ------------------------------------------------------------
// - Consumes BM_Command buffer
// - Applies integer scaling to keep pixel-art sharp
// - Centers the logical canvas in the SDL window
// ============================================================

#include <SDL3/SDL.h>
#include "bangerman.h"

void
BM_SDL3_Render(SDL_Renderer *renderer,
               BM_Context   *ctx,
               int           windowWidth,
               int           windowHeight)
{
    if (!renderer || !ctx) return;

    // --------------------------------------------------------
    // 1) Fetch command buffer + logical size
    // --------------------------------------------------------
    BM_CommandView view = {0};
    bm_get_commands(ctx, &view);

    float logicalW = 320.0f;
    float logicalH = 180.0f;
    bm_get_logical_size(&logicalW, &logicalH);

    // --------------------------------------------------------
    // 2) Integer scaling calc (pixel-art friendly)
    // --------------------------------------------------------
    float scaleX = (float)windowWidth  / logicalW;
    float scaleY = (float)windowHeight / logicalH;
    float scale  = (scaleX < scaleY ? scaleX : scaleY);
    if (scale < 1.0f) scale = 1.0f;
    int intScale = (int)scale;
    if (intScale < 1) intScale = 1;

    float canvasW = logicalW * (float)intScale;
    float canvasH = logicalH * (float)intScale;

    float offsetX = ((float)windowWidth  - canvasW) * 0.5f;
    float offsetY = ((float)windowHeight - canvasH) * 0.5f;

    // --------------------------------------------------------
    // 3) Clear with BangerMan clear color
    // --------------------------------------------------------
    BM_Color clear = bm_get_clear_color();
    SDL_SetRenderDrawColor(
        renderer,
        (Uint8)(clear.r * 255.0f),
        (Uint8)(clear.g * 255.0f),
        (Uint8)(clear.b * 255.0f),
        (Uint8)(clear.a * 255.0f)
    );
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // --------------------------------------------------------
    // 4) Replay commands
    // --------------------------------------------------------
    for (int i = 0; i < view.count; ++i) {
        BM_Command *cmd = &view.commands[i];
        BM_Color c = cmd->color;

        SDL_SetRenderDrawColor(
            renderer,
            (Uint8)(c.r * 255.0f),
            (Uint8)(c.g * 255.0f),
            (Uint8)(c.b * 255.0f),
            (Uint8)(c.a * 255.0f)
        );

        switch (cmd->type) {
        case BM_CMD_RECT_FILL: {
            SDL_FRect r;
            r.x = offsetX + cmd->x * (float)intScale;
            r.y = offsetY + cmd->y * (float)intScale;
            r.w =        cmd->w * (float)intScale;
            r.h =        cmd->h * (float)intScale;
            SDL_RenderFillRect(renderer, &r);
        } break;

        case BM_CMD_RECT_OUTLINE: {
            SDL_FRect r;
            r.x = offsetX + cmd->x * (float)intScale;
            r.y = offsetY + cmd->y * (float)intScale;
            r.w =        cmd->w * (float)intScale;
            r.h =        cmd->h * (float)intScale;
            SDL_RenderRect(renderer, &r);
        } break;

        case BM_CMD_LINE: {
            float x0 = offsetX + cmd->x  * (float)intScale;
            float y0 = offsetY + cmd->y  * (float)intScale;
            float x1 = offsetX + cmd->x2 * (float)intScale;
            float y1 = offsetY + cmd->y2 * (float)intScale;
            SDL_RenderLine(renderer, x0, y0, x1, y1);
        } break;

        case BM_CMD_SPRITE:
            // TODO: sprite rendering will come later.
            break;

        default:
            // Unknown command type, ignore.
            break;
        }
    }
}
