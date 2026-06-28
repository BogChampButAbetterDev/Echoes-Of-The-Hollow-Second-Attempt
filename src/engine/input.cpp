#include "input.h"
#include "globals/globals.h" // USING_CONTROLLER macro

static const int     DEAD_ZONE       = 8000;
static const float   AXIS_NORMALIZE  = 32767.0f;

void pollInput(Input& input, bool& running)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT) running = false;

        if (e.type == SDL_CONTROLLERDEVICEADDED && !input.controller)
            input.controller = SDL_GameControllerOpen(e.cdevice.which);
        
        if (e.type == SDL_CONTROLLERDEVICEREMOVED && input.controller)
        {
            SDL_GameControllerClose(input.controller);
            input.controller = nullptr;
        }
    }

    // if build expects a controller but doesn't get one, set the 
    // "no controller" warning flag
    #if USING_CONTROLLER
        input.noControllerWarning = (input.controller == nullptr);
    #else 
        input.noControllerWarning = false;
    #endif

    // --- keyboard ---
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    bool kb_w        = keys[SDL_SCANCODE_W];
    bool kb_a        = keys[SDL_SCANCODE_A];
    bool kb_s        = keys[SDL_SCANCODE_S];
    bool kb_d        = keys[SDL_SCANCODE_D];
    bool kb_interact = keys[SDL_SCANCODE_SPACE];
    bool kb_escape   = keys[SDL_SCANCODE_ESCAPE];
    bool kb_attack   = keys[SDL_SCANCODE_J];

    // --- controller ---
    bool  gc_w = false, gc_a = false, gc_s = false, gc_d = false;
    bool  gc_interact = false, gc_escape = false, gc_attack = false;
    bool gc_start = false;
    float gc_axisX = 0.0f, gc_axisY = 0.0f;

    if (input.controller)
    {
        // d-pad (digital, contributes to WASD bools)
        gc_w = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        gc_s = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        gc_a = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        gc_d = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

        // left stick — raw axes
        Sint16 rawX = SDL_GameControllerGetAxis(input.controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 rawY = SDL_GameControllerGetAxis(input.controller, SDL_CONTROLLER_AXIS_LEFTY);

        // apply dead zone
        float lx = (std::abs(rawX) > DEAD_ZONE) ? rawX / AXIS_NORMALIZE : 0.0f;
        float ly = (std::abs(rawY) > DEAD_ZONE) ? rawY / AXIS_NORMALIZE : 0.0f;

        gc_axisX = lx;
        gc_axisY = ly;

        // stick also drives the digital bools so 4-dir movement works either way
        if (ly < -0.25f) gc_w = true;
        if (ly >  0.25f) gc_s = true;
        if (lx < -0.25f) gc_a = true;
        if (lx >  0.25f) gc_d = true;

        gc_interact = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_A);
        gc_attack   = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_B);
        gc_escape   = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_GUIDE);
        gc_start    = SDL_GameControllerGetButton(input.controller, SDL_CONTROLLER_BUTTON_START);
    }

    // --- merge ---
    input.w        = kb_w        || gc_w;
    input.a        = kb_a        || gc_a;
    input.s        = kb_s        || gc_s;
    input.d        = kb_d        || gc_d;
    input.interact = kb_interact || gc_interact;
    input.escape   = kb_escape   || gc_escape;
    input.attack   = kb_attack   || gc_attack;
    input.start    = gc_start;

    // keyboard drives axis too so raycasting works without a controller
    if (!input.controller)
    {
        input.axisX = kb_d ? 1.0f : (kb_a ? -1.0f : 0.0f);
        input.axisY = kb_s ? 1.0f : (kb_w ? -1.0f : 0.0f);
    }
    else
    {
        input.axisX = gc_axisX;
        input.axisY = gc_axisY;
    }
}
