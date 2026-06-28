#pragma 
#include <SDL2/SDL.h>
#include <memory>
#include <vector>
#include "ui_screen.h"

class UIManager
{
public:
    void pushScreen(std::unique_ptr<UIScreen> screen) {stack.push_back(std::move(screen));} 
    void popScreen() {if (!stack.empty()) stack.pop_back();}

    bool hasOpenScreen() const {return !stack.empty();}

    // void handleInput(const Cursor& input)
    // {
    //     if (stack.empty()) return;
    //     stack.back()->handleInput(input);
    //     if (stack.back()->wantsToClose())
    //         popScreen();
    // }

    void render(SDL_Renderer* ren)
    {
        for (auto& screen : stack) screen->render(ren);
    }

    std::vector<std::unique_ptr<UIScreen>> stack;
};
