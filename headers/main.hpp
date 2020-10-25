#include <string>
#include <vector>
#include <direct.h>
#include "methods.hpp"
#include "errors.hpp"

namespace ext {
    const std::string data =        "gdit";
    const std::string level =       "gditl";
    const std::string leveldata =   "gditd";
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