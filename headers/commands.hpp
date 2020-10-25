#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "headers.hpp"

namespace commands {
    void parse(int comc, char* com[]) {
        if (comc < 2) {
            std::cout << "Use \"./gdit.exe help\" for help." << std::endl;
            return;
        }
        std::vector<std::string> args;
        for (int i = 0; i < comc; i++)
            args.assign(com + 1, com + comc);
        
        if (args[0] == "init") {
            if (comc < 3)
                std::cout << "You need to provide a level name to initalize GDit in. (Replace spaces in the name with underlines!)" << std::endl;
            else {
                std::string level_name = args[1];
                std::string level = methods::replace(level_name, "_", " ");

                gd::decode::GetCCLocalLevels();

                std::string err;
                std::string lvl = gd::levels::GetLevel(level, &err);

                if (lvl == "")
                    std::cout << err << std::endl;
                else {
                    std::string path = app::dir::main + "\\" + methods::lower(level_name);
                    if (methods::fexists(path))
                        std::cout << "Level has already been initialized!" << std::endl;
                    else {
                        std::cout << "Initializing..." << std::endl;

                        if (_mkdir(path.c_str()) != 0)
                            std::cout << "Couldn't initialize level! Error: " << ERR_COULD_NOT_MAKE_DIR << std::endl;
                        else {
                            std::string fpath = path + "\\" + methods::lower(level_name) + "-master\\" + methods::lower(level_name) + "." + ext::master + ".";
                            
                            if (_mkdir((path + "\\" + methods::lower(level_name) + "-master").c_str()) != 0)
                                std::cout << "Couldn't initialize level! Error: " << ERR_COULD_NOT_MAKE_DIR << std::endl;
                            else {
                                methods::fsave(fpath + ext::leveldata, gd::levels::GetKey(lvl, "k4"));
                                methods::fsave(fpath + ext::levelinfo, gd::levels::WithoutKey(lvl, "k4"));
                                methods::fsave(fpath + "og." + ext::level, lvl);
                                methods::fsave(fpath + ext::main, gdit::GenerateGDitLevelInfo(lvl).dump());
                            }

                            std::cout << "Succesfully initialized " << level << " in " << methods::workdir() << "\\" << fpath + ext::main << " !" << std::endl;
                        }
                    }
                }
            }
        } else if (args[0] == "get") {
            std::string lvl;
            if (comc < 3) {
                std::cout << "Select a gdit to get the level from:" << std::endl;
                for (std::string repo : gdit::GetAllRepos())
                    std::cout << " * " << repo << std::endl;
                std::cin >> lvl;
            } else lvl = args[1];
        } else {
            std::cout << "Unknown command. Use \"./gdit.exe help\" for a list of all commands." << std::endl;
            return;
        }
    }
}