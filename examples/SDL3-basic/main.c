// examples/SDL3-basic/main.c

#include <SDL3/SDL.h>

#define BANGERMAN_IMPLEMENTATION
#include "../../bangerman.h"
#include "../../backends/SDL3/bm_renderer_SDL3.c"

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("BangerMan SDL3 Basic",
                                          800, 600,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    BM_Context *bm = bm_create(1024);
    bm_make_current(bm);
    bm_set_logical_size(320.0f, 180.0f);
    bm_set_clear_color(bm_color_rgba(0.05f, 0.05f, 0.1f, 1.0f));

    BM_SDL3Renderer bmRenderer = {0};
    bmRenderer.renderer = renderer;

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        bm_begin_frame();

        // Red box
        bm_set_draw_color(bm_color_rgb(1.0f, 0.0f, 0.0f));
        bm_rect_fill(10.0f, 10.0f, 50.0f, 30.0f);

        // Green outline
        bm_set_draw_color(bm_color_rgb(0.0f, 1.0f, 0.0f));
        bm_rect_outline(80.0f, 40.0f, 80.0f, 60.0f);

        // White line
        bm_set_draw_color(bm_color_rgb(1.0f, 1.0f, 1.0f));
        bm_line(0.0f, 0.0f, 319.0f, 179.0f);

        bm_end_frame();

        BM_SDL3_Render(&bmRenderer, bm);
        SDL_RenderPresent(renderer);
    }

    bm_destroy(bm);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
