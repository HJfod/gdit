#include <iostream>
#include "headers/commands.hpp"
#ifdef GUI
    #include "gui/gui.hpp"
    #undef main
#endif
//#include <io.h>
//#include <fcntl.h>

int main(int argc, char *argv[]) {
    //_setmode(_fileno(stdout), _O_U16TEXT);
    
    // load settings and create required directories
    app::settings::load();
    app::dir::init();

    // handle command line input
    commands::parse(argc, argv);

    #ifdef GUI
        if (argc < 2 || strcmp(argv[1], "debug")) {
            HWND windowHandle = GetConsoleWindow();
            ShowWindow(windowHandle, SW_HIDE);
        }
        gui::init();
    #endif

    return 0;
}