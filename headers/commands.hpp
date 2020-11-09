#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "headers.hpp"

constexpr unsigned int $(const char* str, int h = 0) {
    return !str[h] ? 5381 : ($(str, h+1) * 33) ^ str[h];
}

namespace commands {
    namespace g {
        void import(std::string _path) {
            if (app::settings::sval("username") == "") {
                std::cout << "What's your GD name? (no spaces): ";
                std::string u;
                std::cin >> u;
                app::settings::sset("username", u);
            }
            bool endan = false;
            std::thread l = console::showload("Importing...", &endan);
            int x = gdit::AddGditPart(_path, app::settings::sval("username"));
            endan = true;
            l.join();
            if (x == GDIT_IMPORT_SUCCESS)
                std::cout << "Succesfully imported part! You can now start working on it :)" << std::endl << "NOTE: DO *NOT* CHANGE THE NAME OF THE LEVEL." << std::endl;
            else std::cout << "Error: " << x << std::endl;
        }
        
        void init(std::string _name) {
            std::string level_name = _name;
            std::string level = methods::replace(level_name, "_", " ");

            gd::decode::GetCCLocalLevels();

            std::string err;
            rapidxml::xml_node<>* lvl = gd::levels::GetLevel(level, &err);

            if (lvl == NULL)
                std::cout << err << std::endl;
            else {
                std::string path = app::dir::main + "\\" + methods::lower(level_name);
                if (methods::fexists(path))
                    std::cout << "Level has already been initialized!" << std::endl;
                else {
                    bool endan = false;
                    std::thread l = console::showload("Initializing...", &endan);

                    std::string fpath;
                    int er = gdit::InitGdit(level_name, lvl, &fpath);
                    
                    endan = true;
                    l.join();

                    if (er != GDIT_INIT_SUCCESS)
                        std::cout << "Couldn't initialize level! Error: " << er << std::endl;
                    else
                        std::cout << "Succesfully initialized " << level << " in " << methods::workdir() << "\\" << fpath + ext::main << " !" << std::endl;
                }
            }
        }
        
        void get(std::string _name = "") {
            std::string lvl;
            if (_name == "") {
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
            } else lvl = _name;

            if (lvl == "") return;
            if (gdit::GditExists(lvl)) {
                std::cout << gdit::GetGditLevel(lvl) << std::endl;
                std::cout << "Send this file to your collab participants! :)" << std::endl;
            } else std::cout << "gdit not found! Use \"gdit init <level>\" to create a gdit." << std::endl;
        }

        void commit(std::string _name = "", std::string _flags = "") {
            std::string lvl;
            if (_name == "") {
                std::vector<std::string> repos = gdit::GetAllRepos(GDIT_TYPE_PART);
                if (repos.size() == 0)
                    std::cout << "You have no gdit parts! Use \"gdit <path-to-file>\" to begin working on a gdit part." << std::endl;
                else {
                    std::cout << "Select a gdit part to commit:" << std::endl;
                    std::string res;
                    int s = console::selectmenu(repos, &res);

                    if (s == -1)
                        std::cout << "Cancelled selection" << std::endl;
                    else
                        lvl = res;
                }
            } else lvl = _name;

            if (lvl == "") return;
            if (gdit::GditExists(lvl)) {
                struct flags {
                    bool create_anyway = false;
                };
                flags f = {};
                if (_flags != "")
                    f.create_anyway = (_flags.find("-create") != std::string::npos);
                bool end = false;
                std::string res;
                std::thread l = console::showload("Detecting changes... (this may take a while)", &end);
                int r = gdit::CommitChanges(lvl, &res, f.create_anyway);
                end = true;
                l.join();
                if (r == GDIT_COMMIT_SUCCESS)
                    std::cout << "Commit succesful!" << std::endl << res << std::endl;
                else std::cout << "Error committing: " << r << std::endl;
            } else std::cout << "gdit part not found!" << std::endl;
        }

        void merge(std::string _part) {
            if (_part == "") return;
            
            std::vector<std::string> parts;
            if (methods::ewith(_part, ext::commit)) parts = { _part };
            else parts = methods::dall(_part);

            std::string name = gdit::GetGDitNameFromCommit(parts[0]);
            if (!gdit::GditExists(name)) {
                std::cout << "GDit not found! (Are you not the megacollab host?)" << std::endl;
                return;
            }

            for (std::string part : parts) {
                if (gdit::GetGDitNameFromCommit(part) != name) continue;

                int m = gdit::MergeCommit(_part);
                
                if (m == GDIT_MERGE_SUCCESS)
                    std::cout << "Succesfully merged!" << std::endl;
                else std::cout << "Error merging: " << m << std::endl;
            }
        }

        void view(std::string _name = "", std::string _flags = "") {
            std::string lvl;
            if (_name == "") {
                std::vector<std::string> repos = gdit::GetAllRepos();
                if (repos.size() == 0)
                    std::cout << "You have no gdits! Use \"gdit init <level>\" to start a new gdit." << std::endl;
                else {
                    std::cout << "Select a gdit to view:" << std::endl;
                    std::string res;
                    int s = console::selectmenu(repos, &res);

                    if (s == -1)
                        std::cout << "Cancelled selection" << std::endl;
                    else
                        lvl = res;
                }
            } else lvl = _name;

            if (lvl == "") return;
            if (gdit::GditExists(lvl)) {
                //bool endan = false;
                //std::thread l = console::showload("Loading...", &endan);
                gdit::ViewGditLevel(lvl, _flags.find("-og") != std::string::npos);
                //endan = true;
                //l.join();
                std::cout << "Added to your GD levels!" << std::endl;
            } else std::cout << "gdit not found! Use \"gdit init <level>\" to create a gdit." << std::endl;
        }

        void setup(std::string _var = "", std::string _val = "") {
            if (_var == "")
                std::cout << "Usage: setup <variable> [<value>]" << std::endl << "Variables:\n" << app::settings::all();
            else if (_val == "")
                std::cout << (app::settings::sval(_var) == "" ? "<No value set>" : app::settings::sval(_var)) << std::endl;
            else
                if (app::settings::sset(_var, _val) != GDIT_SETTING_UPDATE_SUCCESS)
                    std::cout << "Unable to update setting!" << std::endl;
                else std::cout << "Succesfully updated setting!" << std::endl;
        }
    }

    void parse(int comc, char* com[]) {
        if (comc < 2) {
            std::cout << "Use \"./gdit.exe help\" for help." << std::endl;
            return;
        }
        std::vector<std::string> args;
        for (int i = 0; i < comc; i++)
            args.assign(com + 1, com + comc);
        
        if (args[0].find("\\", 0) != std::string::npos || args[0].find("/", 0) != std::string::npos) {
            if (methods::ewith(args[0], ext::level))
                g::import(args[0]);
        } else
        switch ($(methods::lower(args[0]).c_str())) {
            #pragma region init
            case $("init"):
                {
                    if (comc < 3)
                        std::cout << "You need to provide a level name to initalize GDit in. (Replace spaces in the name with underlines!)" << std::endl;
                    else g::init(args[1]);
                }
                break;
            #pragma endregion
            #pragma region get
            case $("get"):
                {
                    g::get(comc < 3 ? "" : args[1]);
                }
                break;
            #pragma endregion get
            #pragma region commit
            case $("commit"):
                {
                    g::commit(comc < 3 ? "" : args[1], comc < 4 ? "" : args[2]);
                }
                break;
            case $("com_reset"):
                {
                    if (comc < 3) std::cout << "You need to supply a gdit part to erase commits in!" << std::endl;
                    else {
                        int g = gdit::ResetCommits(args[1]);
                        if (g == GDIT_COMMIT_SUCCESS) std::cout << "Succesfully reset commits!";
                        else std::cout << "Error resetting: " << g << std::endl;
                    }
                }
                break;
            #pragma endregion commit
            #pragma region setup
            case $("setup"):
                {
                    g::setup(comc < 3 ? "" : args[1], comc < 4 ? "" : args[2]);
                }
                break;
            #pragma endregion setup
            #pragma region merge
            case $("merge"):
                {
                    if (comc < 3) std::cout << "You need to supply a path to a ." << ext::commit << " file to merge!" << std::endl;
                    else g::merge((args[1] == "from" ? (comc < 4 ? "" : args[2]) : args[1]));
                }
                break;
            case $("view"):
                {
                    g::view(comc < 3 ? "" : args[1], comc > 3 ? args[2] : "");
                }
                break;
            #pragma endregion merge
            #pragma region debug
            case $("debug_dcc"):
                {
                    methods::fsave("ccl.txt", gd::decode::DecodeCCLocalLevels());
                }
                break;
            case $("debug_dcc_xml"):
                {
                    methods::fsave("ccl.txt", methods::xts(gd::decode::GetCCLocalLevels()));
                }
                break;
            #pragma endregion debug
            #pragma region default
            default:
                std::cout << "Unknown command. Use \"./gdit.exe help\" for a list of all commands." << std::endl;
            #pragma endregion default
        }
    }
}