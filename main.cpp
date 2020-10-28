#include <iostream>
#include "headers/commands.hpp"
//#include <io.h>
//#include <fcntl.h>

int main(int argc, char *argv[]) {
    //_setmode(_fileno(stdout), _O_U16TEXT);

    app::settings::load();
    app::dir::init();

    commands::parse(argc, argv);

    return 0;
}