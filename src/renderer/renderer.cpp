#include "renderer.h"

#ifdef __3DS__
Render::Render(int dummy)
{
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    top    = C2D_CreateScreenTarget(GFX_TOP,    GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
}

void Render::beginFrame()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
    C2D_SceneBegin(top);
}

void Render::presentFrame()
{
    C3D_FrameEnd(0);
}

void Render::renderFromQueue()
{
    std::sort(r_queue.begin(), r_queue.end(),
              [](const RenderRequest& a, const RenderRequest& b)
              {
                return a.z < b.z;
              });
    for (RenderRequest& rq : r_queue)
    {
        if (!rq.c2dImage.tex) continue;

        C2D_DrawImageAt(rq.c2dImage,
            (float)rq.x, (float)rq.y,
            0.0f, //depth
            nullptr,
            (float)rq.w / rq.c2dImage.subtex->width,
            (float)rq.h / rq.c2dImage.subtex->height);
    }
    r_queue.clear();
}

#else
Render::Render(SDL_Window* win)
{
    renderer = createRen(win);
}

void Render::beginFrame()
{
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer);
}

void Render::presentFrame()
{
    SDL_RenderPresent(this->renderer);
}

void Render::renderFromQueue()
{
    // thing with lowest z renders first
    std::sort(r_queue.begin(), r_queue.end(),
              [](const RenderRequest& a, const RenderRequest& b)
              {
                return a.z < b.z;
              });

    for (RenderRequest& rq : r_queue)
    {
        SDL_Rect dst = 
        {
            rq.x, 
            rq.y, 
            rq.w,
            rq.h
        };
        SDL_RenderCopy(this->renderer, rq.tex, &rq.src, &dst);
    }

    r_queue.clear();
}

SDL_Renderer *Render::createRen(SDL_Window* win)
{
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        std::cout << "cannot create renderer\n";
        exit(1);
    }
    return ren;
}
#endif
