#include <iostream>
#include <Windows.h>
#include "wind.hpp"

namespace gui {
    void init() {
        int SDL_Init(SDL_INIT_VIDEO);
        SDL_VideoInit(NULL);

        gdit_window* w_main = new gdit_window;
        w_main->init(GDIT_APP_TITLE);
    }

    void close() {
        SDL_Quit();
    }
}