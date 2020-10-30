#include <iostream>
#include <Windows.h>
#include "wind.h"

namespace gui {
    void init() {
        gdit_window* main = new gdit_window("main");
    }

    void close() {
        SDL_Quit();
    }
}