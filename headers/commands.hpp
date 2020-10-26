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
                            std::cout << "Couldn't initialize level! Error: " << GDIT_COULD_NOT_MAKE_DIR << std::endl;
                        else {
                            std::string fpath = path + "\\" + "master\\" + methods::lower(level_name) + "." + ext::master + ".";
                            
                            if (_mkdir((path + "\\" + "master").c_str()) != 0)
                                std::cout << "Couldn't initialize level! Error: " << GDIT_COULD_NOT_MAKE_DIR << std::endl;
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
                std::vector<std::string> repos = gdit::GetAllRepos();
                if (repos.size() == 0)
                    std::cout << "You have no gdits! Use \"gdit init <level>\" to start a new gdit." << std::endl;
                else {
                    std::cout << "Select a gdit to get the level from:" << std::endl;
                    std::string res;
                    int s = console::selectmenu(repos, &res);

                    if (s == -1)
                        std::cout << "Cancelled selection" << std::endl;
                    else
                        lvl = res;
                }
            } else lvl = args[1];

            if (gdit::GditExists(lvl)) {
                std::cout << gdit::GetGditLevel(lvl) << std::endl;
                std::cout << "Send this file to your collab participants! :)" << std::endl;
            } else std::cout << "gdit not found! Use \"gdit init <level>\" to create a gdit." << std::endl;
        } else if (args[0].find("\\", 0) != std::string::npos || args[0].find("/", 0) != std::string::npos) {
            if (methods::ewith(args[0], ext::level))
                gdit::AddGditPart(args[0]);
        } else {
            std::cout << "Unknown command. Use \"./gdit.exe help\" for a list of all commands." << std::endl;
            return;
        }
    }
}