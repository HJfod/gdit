#include <iostream>
#include "headers/commands.hpp"

int main(int argc, char *argv[]) {
    app::settings::load();
    app::dir::init();

    commands::parse(argc, argv);

    return 0;
}