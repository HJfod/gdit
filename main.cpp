#include <iostream>
#include "headers/commands.hpp"
#ifdef GUI
    #include "gui/gui.hpp"
    #undef main
#endif

int main(int argc, char *argv[]) {
    app::settings::load();
    app::dir::init();

    commands::parse(argc, argv);

    #ifdef GUI
        if (argc < 2 || strcmp(argv[1], "debug")) {
            HWND windowHandle = GetConsoleWindow();
            ShowWindow(windowHandle, SW_HIDE);

            gui::init();

            SDL_Event event;

            while (!gui::sett::quit) {
                if (SDL_PollEvent(&event)) {
                    if(event.type == SDL_QUIT)
                        gui::sett::quit = true;
                    if (event.type == SDL_TEXTINPUT)
                        if (std::string(event.text.text) == "n") {
                            gui::gdit_window* w_ = new gui::gdit_window;
                            w_->init("__w__");
                        }
                    for (gui::gdit_window* w : gui::windows::all)
                        w->handle(event);
                }

                for (gui::gdit_window* w : gui::windows::all)
                    if (w->update) w->render();
            }
        }
    #endif

    return 0;
}