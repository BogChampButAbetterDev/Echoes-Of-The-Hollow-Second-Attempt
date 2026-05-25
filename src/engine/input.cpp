#include "input.h"

void pollInput(Input& input, bool& running)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT) running = false;
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    input.w         = keys[SDL_SCANCODE_W];
    input.a         = keys[SDL_SCANCODE_A];
    input.s         = keys[SDL_SCANCODE_S];
    input.d         = keys[SDL_SCANCODE_D];
    input.interact  = keys[SDL_SCANCODE_SPACE];
    input.escape    = keys[SDL_SCANCODE_ESCAPE];
}
