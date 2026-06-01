#include "ren_queue.h"

std::vector<RenderRequest> r_queue;

void submit(SDL_Texture* tex, SDL_Rect src, int x, int y, int w, int h, bool flipH)
{
    r_queue.push_back({tex, src, x, y, w, h, y + h, flipH});
}
