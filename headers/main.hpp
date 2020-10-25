#include <string>
#include <vector>
#include <direct.h>
#include "methods.hpp"
#include "errors.hpp"

namespace ext {
    const std::string main =        "gdit";
    const std::string level =       "gditl";
    const std::string levelinfo =   "gditi";
    const std::string leveldata =   "gditd";
    const std::string master =      "master";
}

namespace app {
    namespace dir {
        const std::string main = "gdit";

        void init() {
            if (!methods::fexists(app::dir::main))
                _mkdir(app::dir::main.c_str());
        }
    }
    std::string decoded_data;
    std::vector<std::string> levels;
}