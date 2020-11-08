#include <string>
#include <vector>
#include <direct.h>
#include "methods.hpp"
#include "../ext/json.hpp"

namespace ext {
    const std::string main =        "gdit";
    const std::string level =       "gditl";
    const std::string levelinfo =   "gditi";
    const std::string leveldata =   "gditd";
    const std::string commit =      "gditc";
    const std::string master =      "master";
}

namespace app {
    namespace dir {
        const std::string main = "gdit";
        const std::string copies = "copies";
        const std::string user = "user";
        const std::string userfile = "user\\user." + ext::main;

        void init() {
            if (!methods::fexists(app::dir::main))
                _mkdir(app::dir::main.c_str());
            if (!methods::fexists(app::dir::copies))
                _mkdir(app::dir::copies.c_str());
            if (!methods::fexists(app::dir::user))
                _mkdir(app::dir::user.c_str());
        }
    }
    namespace settings {
        struct setting {
            std::string name;
            std::string val;
        };
        
        std::vector<setting> settings = {
            { "username", "" },
            { "test", "" }
        };

        int sget(std::string _set) {
            int i = 0;
            for (setting s : settings)
                if (s.name == _set) return i; else i++;
            return -1;
        }

        std::string sval(std::string _set) {
            return sget(_set) == -1 ? "" : settings[sget(_set)].val;
        }

        int sset(std::string _set, std::string _val) {
            int ix = sget(_set);
            if (ix == -1) return GDIT_SETTING_DOESNT_EXIST;
            nlohmann::json data = {};
            if (methods::fexists(dir::userfile))
                data = nlohmann::json::parse(methods::fread(dir::userfile));
            data[settings[ix].name] = _val;
            methods::fsave(dir::userfile, data.dump());
            settings[ix].val = _val;
            return GDIT_SETTING_UPDATE_SUCCESS;
        }

        std::string all() {
            std::string ret;
            for (setting s : settings)
                ret += s.name + "\n";
            return ret;
        }
        
        void load() {
            if (methods::fexists(dir::userfile)) {
                nlohmann::json j = nlohmann::json::parse(methods::fread(dir::userfile));
                for (auto x = j.begin(); x != j.end(); ++x)
                    for (int i = 0; i < settings.size(); i++)
                        if (settings[i].name == x.key())
                            settings[i].val = methods::sanitize(x.value());
            }
        }
    }
    rapidxml::xml_document<> decoded_data;
    std::vector<rapidxml::xml_node<>*> levels;
}