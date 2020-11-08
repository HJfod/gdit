#include <string>
#include <ShlObj.h>
#include <windows.h>
#include <vector>
#include <fstream>
#include <regex>
#include <algorithm>
#include <stdio.h>
#include <chrono>
#include "main.hpp"
#include "../ext/ZlibHelper.hpp"
#include "../ext/Base64.hpp"
#include "../ext/dirent.h"
#include "../ext/json.hpp"

namespace gd {
    namespace decode {
        std::string GetCCPath(std::string WHICH) {
            wchar_t* localAppData = 0;
            SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppData);

            std::wstring CCW (localAppData);

            std::string RESULT ( CCW.begin(), CCW.end() );
            RESULT += "\\GeometryDash\\CC" + WHICH + ".dat";

            CoTaskMemFree(static_cast<void*>(localAppData));
            
            return RESULT;
        }

        std::vector<uint8_t> readf(std::string const& path) {
            std::vector<uint8_t> buffer;
            std::ifstream file(path, std::ios::ate, std::ios::binary);

            if (file.is_open()) {
                buffer.resize(file.tellg());
                file.seekg(0, std::ios::beg);

                file.read(
                    reinterpret_cast<char*>(buffer.data()),
                    buffer.size());
            }

            return buffer;
        }
        
        void DecodeXOR(std::vector<uint8_t>& BYTES, int KEY) {
            for (auto& b : BYTES)
                b ^= KEY;
        }

        std::vector<uint8_t> DecodeBase64(const std::string& str) {
            gdcrypto::base64::Base64 b64(gdcrypto::base64::URL_SAFE_DICT);
            return b64.decode(str);
        }

        std::string DecompressGZip(const std::vector<uint8_t> str) {
            auto buffer = gdcrypto::zlib::inflateBuffer(str);
            return std::string(buffer.data(), buffer.data() + buffer.size());
        }

        std::string DecodeCCLocalLevels() {
            std::string CCPATH = decode::GetCCPath("LocalLevels");
            std::vector<uint8_t> CCCONTENTS = decode::readf(CCPATH);

            std::string c = methods::fread(CCPATH);
            if (c._Starts_with("<?xml version=\"1.0\"?>")) {
                app::decoded_data.parse<0>(methods::stc(c));
                return c;
            }

            DecodeXOR(CCCONTENTS, 11);
            auto XOR = std::string(CCCONTENTS.begin(), CCCONTENTS.end());
            std::vector<uint8_t> B64 = DecodeBase64(XOR);
            std::string ZLIB = DecompressGZip(B64);

            return ZLIB;
        }

        bool SaveCCLocalLevels() {
            methods::fsave(decode::GetCCPath("LocalLevels"), "<?xml version=\"1.0\"?>\n" + methods::xts(&app::decoded_data));

            return true;
        }

        rapidxml::xml_document<>* GetCCLocalLevels() {
            if (app::decoded_data.first_node() == 0)
                app::decoded_data.parse<0>(methods::stc(DecodeCCLocalLevels()));
            return &app::decoded_data;
        }

        std::string DecodeLevelData(std::string _data) {
            return DecompressGZip(DecodeBase64(_data));
        }
    }

    namespace levels {
        void LoadLevels() {
            if (app::levels.size() == 0) {
                gd::decode::GetCCLocalLevels();

                rapidxml::xml_node<>* d = app::decoded_data.first_node("plist")->first_node("dict")->first_node("d");
                rapidxml::xml_node<>* fs = NULL;
                
                std::vector<rapidxml::xml_node<>*> LIST = {};
                for (rapidxml::xml_node<>* child = d->first_node(); child; child = child->next_sibling())
                    if (std::strcmp(child->name(), "d") == 0)
                        LIST.push_back(child);

                app::levels = LIST;
            }
        }

        std::string GetKey_X(rapidxml::xml_node<>* _lvl, const char* _key) {
            for (rapidxml::xml_node<>* child = _lvl->first_node(); child; child = child->next_sibling())
                if (std::strcmp(child->name(), "k") == 0)
                    if (std::strcmp(child->value(), _key) == 0)
                        return child->next_sibling()->value();
            return "";
        }

        std::string SetKey(std::string *_data, std::string _key, std::string _val) {
            int p = _data->find("<k>" + _key + "</k>") - 7 - _key.length();

            std::string m = _data->substr(_data->find("<k>" + _key + "</k>") + 7 + _key.length());
            std::string t = m.substr(1, 1);
            std::string r = "<k>" + _key + "</k><" + t + ">" + _val + "</" + t + ">";

            *_data = std::regex_replace(*_data, std::regex(R"(<k>)" + _key + R"(<\/k><.>.*?<\/.>)"), r, std::regex_constants::match_any);
            return r;
        }

        bool SetKey_X(rapidxml::xml_node<>* _lvl, const char* _key, const char* _val, const char* _type = "s") {
            for (rapidxml::xml_node<>* child = _lvl->first_node(); child; child = child->next_sibling())
                if (std::strcmp(child->name(), "k") == 0)
                    if (std::strcmp(child->value(), _key) == 0) {
                        child->next_sibling()->first_node()->value(_val);
                        return true;
                    }
            std::string n_k ("<k>" + std::string (_key) + "</k><" + std::string (_type) + ">" + std::string (_val) + "</" + std::string (_type) + ">");
            rapidxml::xml_document<> n;
            n.parse<0>(methods::stc(n_k));

            rapidxml::xml_node<>* $k = _lvl->document()->clone_node(n.first_node("k"));
            rapidxml::xml_node<>* $t = _lvl->document()->clone_node(n.first_node(_type));
            _lvl->append_node($k);
            _lvl->append_node($t);
            return true;
        }

        std::string WithoutKey(const std::string DATA, std::string KEY) {
            std::regex m ("<k>" + KEY + "</k><.>");
            std::smatch cm;
            std::regex_search(DATA, cm, m);

            if (cm[0] == "") return "";

            std::string T_TYPE = ((std::string)cm[0]).substr(((std::string)cm[0]).find_last_of("<") + 1, 1);

            std::regex tm ("<k>" + KEY + "</k><" + T_TYPE + ">.*?</" + T_TYPE + ">");
            std::smatch tcm;
            std::regex_search(DATA, tcm, tm);

            std::string VAL = tcm[0];

            int L1 = ("<k>" + KEY + "</k><" + T_TYPE + ">").length();
            return DATA.substr(0, L1) + DATA.substr(VAL.find_last_of("</") - L1 - 1);
        }

        rapidxml::xml_node<>* GetLevel(std::string _name, std::string *_err) {
            LoadLevels();

            for (int i = 0; i < app::levels.size(); i++)
                if (methods::lower(GetKey_X(app::levels[i], "k2")) == methods::lower(_name))
                    return app::levels[i];

            *_err = "Could not find level! (Replace spaces in name with _)";
            return NULL;
        }

        int ImportLevel_X(std::string _path, std::string _lvl = "", std::string _name = "") {
            std::string lvl;
            if (_lvl == "")
                lvl = methods::fread(_path);
            else lvl = _lvl;

            gd::decode::GetCCLocalLevels();

            rapidxml::xml_node<>* d = app::decoded_data.first_node("plist")->first_node("dict")->first_node("d");
            rapidxml::xml_node<>* fs = NULL;
            for (rapidxml::xml_node<>* child = d->first_node(); child; child = child->next_sibling()) {
                if (std::strcmp(child->name(), "k") == 0)
                    if (std::string(child->value()).find("k_") != std::string::npos) {
                        child->first_node()->value(methods::stc("k_" + std::to_string(std::stoi(std::string(child->value()).substr(2)) + 1)));
                        if (fs == NULL) fs = child;
                    }
            }

            rapidxml::xml_document<> lv;
            lv.parse<0>(methods::stc(lvl));
            SetKey_X(lv.first_node(), "k2", _name.c_str());

            rapidxml::xml_node<>* ln = lv.first_node();
            rapidxml::xml_node<>* lnt = app::decoded_data.clone_node(ln);

            rapidxml::xml_document<> k_ix;
            k_ix.parse<0>(methods::stc("<k>k_0</k>"));
            rapidxml::xml_node<>* lk_ix = app::decoded_data.clone_node(k_ix.first_node());

            d->insert_node(fs, lk_ix);
            d->insert_node(fs, lnt);

            gd::decode::SaveCCLocalLevels();

            return GDIT_IMPORT_SUCCESS;
        }

        struct gd_obj {
            std::string data;
        };

        struct obj_group {
            std::string obj_type;
            std::vector<gd_obj> objs;
        };

        std::vector<gd_obj> GetObjects(std::string _decoded_data, std::vector<obj_group>* _ordered = NULL) {
            std::string d = _decoded_data.substr(_decoded_data.find(";") + 1);
            std::vector<gd_obj> res = {};
            std::vector<obj_group> orres = {};
            while (d.length() > 0) {
                if (d.find(";") == std::string::npos) break;
                std::string obj = d.substr(0, d.find(";"));
                d = d.substr(obj.length() + 1);
                if (obj.empty()) continue;
                res.push_back({ obj });
                if (_ordered != NULL) {
                    std::string id = obj.substr(obj.find_first_of(",") + 1);
                    id = id.substr(0, id.find_first_of(","));
                    int ix = -1, i = 0;
                    for (obj_group gr : orres)
                        if (gr.obj_type == id) ix = i; else i++;
                    if (ix == -1)
                        orres.push_back({ id, { { obj } } });
                    else
                        orres[ix].objs.push_back({ obj });
                }
            }
            *_ordered = orres;
            return res;
        }
    }
}

namespace gdit {
    nlohmann::json GenerateGDitLevelInfo(rapidxml::xml_node<>* _data) {
        std::string song = gd::levels::GetKey_X(_data, "k8");
        return {
            { "name", gd::levels::GetKey_X(_data, "k2") },
            { "song", (song == "") ? ("<k>k45</k><i>" + gd::levels::GetKey_X(_data, "k45") + "</i>") : ("<k>k8</k><i>" + song + "</i>") },
            { "init-time", methods::time() }
        };
    }

    int InitGdit(std::string _name, rapidxml::xml_node<>* _lvl, std::string *_rpath, bool nomaster = false) {
        std::string path = app::dir::main + "\\" + methods::lower(_name);
        if (_mkdir(path.c_str()) != 0)
            return GDIT_COULD_NOT_MAKE_DIR;
        else {
            std::string fpath = path + "\\" + "master\\" + methods::lower(_name) + "." + ext::master + ".";
            
            if (!nomaster)
                if (_mkdir((path + "\\" + "master").c_str()) != 0)
                    return GDIT_COULD_NOT_MAKE_DIR;
                else {
                    methods::fsave(fpath + ext::leveldata, gd::levels::GetKey_X(_lvl, "k4"));
                    //methods::fsave(fpath + ext::levelinfo, gd::levels::WithoutKey(_lvl, "k4"));
                    methods::fsave(fpath + "og." + ext::level, methods::xts(_lvl));
                    methods::fsave(fpath + ext::main, gdit::GenerateGDitLevelInfo(_lvl).dump());

                    *_rpath = fpath;
                }
            else *_rpath = path;

            return GDIT_INIT_SUCCESS;
        }
    }

    bool VerifyDirHasParts (std::string f, int n) {
        return f.substr(f.find_last_of("\\") + 1)._Starts_with("part_");
    }

    bool VerifyDirIsRepo(std::string _dir, int _type) {
        if (_type == GDIT_TYPE_GDIT)
            return methods::fexists(_dir + "\\master");
        return methods::dread(_dir, &VerifyDirHasParts).size() > 0;
    }

    std::vector<std::string> GetAllRepos(int _type = GDIT_TYPE_GDIT) {
        return methods::dread(app::dir::main.c_str(), &VerifyDirIsRepo, _type);
    }

    bool GditExists(std::string _name) {
        return methods::fexists(app::dir::main + "\\" + _name + "\\master") ||
               methods::fexists(app::dir::main + "\\" + _name + "\\parts");
    }

    std::string GetGditLevel(std::string _name) {
        std::string to = methods::workdir() + "\\" + app::dir::copies + "\\" + _name + ".copy." + ext::level;
        methods::fcopy(
            app::dir::main + "\\" + _name + "\\master\\" + _name + ".master.og." + ext::level, to
        );
        return to;
    }
    
    std::string GetGDitNameFromPath(std::string _path) {
        _path = methods::replace(_path, "/", "\\");
        std::string sh = _path.substr(_path.find_last_of("\\", _path.length()) + 1);
        return sh.substr(0, sh.find_first_of(".", 0));
    }

    std::string GetGDitNameFromCommit(std::string _path) {
        if (!methods::fexists(_path)) return "";
        std::string txt = methods::fread(_path);
        txt = txt.substr(txt.find("\nDATA ") + 5);
        return methods::sanitize(nlohmann::json::parse(txt.substr(txt.find_first_of("\n") + 1, 
            std::stoi(txt.substr(txt.find_first_of(" "), txt.find_first_of("\n") - txt.find_first_of(" ")))
        ))["gdit_name"].dump());
    }

    int AddGditPart(std::string _path, std::string _creator) {
        std::string name = GetGDitNameFromPath(_path);
        std::string tp = "";
        std::string rpath;
        std::string gdname = name + "@" + _creator;

        if (GditExists(name))
            tp = app::dir::main + "\\" + name;
        else {
            rapidxml::xml_document<> s;
            s.parse<0>(methods::stc(methods::fread(_path)));
            InitGdit(name, s.first_node(), &rpath, true);
        }
        
        tp = tp == "" ? rpath : tp;

        if (!methods::fexists(tp + "\\part_" + _creator))
            if (_mkdir((tp + "\\part_" + _creator).c_str()) != 0)
                return GDIT_COULD_NOT_MAKE_DIR;
        
        methods::fcopy(
            _path, tp + "\\part_" + _creator + "\\" + name + ".og." + ext::level
        );
        methods::fcopy(
            _path, tp + "\\part_" + _creator + "\\" + name + ".work." + ext::level
        );
        methods::fsave(tp + "\\part_" + _creator + "\\" + name + "." + ext::main, nlohmann::json({
            { "name", gdname }
        }).dump());
        gd::levels::ImportLevel_X(_path, "", gdname);
        return GDIT_IMPORT_SUCCESS;
    }

    int CommitChanges(std::string _gdit, std::string* _out = NULL, bool _c_an = false) {
        if (app::settings::sval("username") == "")
            return GDIT_USERNAME_NOT_SET;

        std::string dir = methods::workdir() + "\\" + app::dir::main + "\\" + _gdit + "\\" + "part_" + app::settings::sval("username");
        
        std::string gd_name = methods::sanitize(nlohmann::json::parse(methods::fread(dir + "\\" + _gdit + "." + ext::main))["name"].dump());

        std::string err;
        rapidxml::xml_node<>* lvl = gd::levels::GetLevel(gd_name, &err);
        if (lvl == NULL) {
            *_out = err;
            return GDIT_LEVEL_DOESNT_EXIST;
        }

        std::string olvls = methods::fread(dir + "\\" + _gdit + ".work." + ext::level);
        if (olvls == "")
            return GDIT_LEVEL_DOESNT_EXIST;
        rapidxml::xml_document<> oldc;
        oldc.parse<0>(methods::stc(olvls));
        rapidxml::xml_node<>* olvl = oldc.first_node();

        methods::fsave(dir + "\\" + _gdit + ".work." + ext::level, methods::xts(lvl));

        std::string new_k4 = gd::levels::GetKey_X(lvl,  "k4");
        std::string og_k4  = gd::levels::GetKey_X(olvl, "k4");

        bool skip = false;
        if (new_k4 == og_k4) {
            if (_out != NULL) *_out = "No changes found.\n";
            if (_c_an) skip = true; else return GDIT_COMMIT_SUCCESS;
        }

        std::vector<std::string> obj_removed = {};
        std::vector<std::string> obj_added = {};

        if (!skip) {
            std::vector<gd::levels::obj_group> objs = {};
            std::string dec_og = gd::decode::DecodeLevelData(og_k4);
            std::string dec_new= gd::decode::DecodeLevelData(new_k4);
            std::vector<gd::levels::gd_obj> new_obj = gd::levels::GetObjects(gd::decode::DecodeLevelData(og_k4), &objs);
            
            /*
            for (gd::levels::obj_group gr : objs) {
                std::cout << gr.obj_type << ":\t [  ";
                for (gd::levels::gd_obj o : gr.objs)
                    std::cout << o.data << "; ";
                std::cout << " ]" << std::endl;
            }
            //*/

            std::string d = dec_new.substr(dec_new.find(";") + 1);
            int obj_count = 0;
            while (d.length() > 0) {
                if (d.find(";") == std::string::npos) break;
                std::string obj = d.substr(0, d.find(";"));
                d = d.substr(obj.length() + 1);
                if (obj.empty()) continue;

                obj_count++;

                std::string id = obj.substr(obj.find_first_of(",") + 1);
                id = id.substr(0, id.find_first_of(","));
                int ix = -1, i = 0;
                for (gd::levels::obj_group gr : objs)
                    if (gr.obj_type == id) ix = i; else i++;
                bool found = false;
                int j = 0;
                if (ix == -1) {
                    obj_added.push_back(obj);
                    continue;
                } else for (gd::levels::gd_obj o : objs[ix].objs)
                    if (o.data == obj) {
                        found = true;
                        objs[ix].objs.erase(objs[ix].objs.begin() + j);
                        break;
                    } else j++;
                if (!found)
                    obj_added.push_back(obj);
            }
            /*
            std::cout << "new level object count:\t" << obj_count << std::endl;
            std::cout << "old level object count:\t" << new_obj.size() << std::endl;
            */
            for (gd::levels::obj_group gr : objs)
                for (gd::levels::gd_obj go : gr.objs)
                        obj_removed.push_back(go.data);
        }

        std::string output_file = "";
        
        nlohmann::json ij;
        ij["type"] = GDIT_COMMIT_VERSION;
        ij["gdit_name"] = _gdit;
        ij["added"] = obj_added;
        ij["removed"] = obj_removed;
        output_file += "VERSION " + methods::sanitize(ij["type"].dump()) + "\n";
        output_file += "DATA " + std::to_string(ij.dump().length()) + "\n" + ij.dump() + "\n";

        methods::fsave(dir + "\\" + methods::time("_") + ".commit." + ext::commit, output_file);
        
        if (_out != NULL && !skip) {
            *_out = "Added objects:  \t" + std::to_string(obj_added.size()) + "\n";
            *_out +="Removed objects:  \t" + std::to_string(obj_removed.size()) + "\n";
        }

        return GDIT_COMMIT_SUCCESS;
    }

    int ResetCommits(std::string _gdit) {
        if (app::settings::sval("username") == "")
            return GDIT_USERNAME_NOT_SET;
        std::string dir = methods::workdir() + "\\" + app::dir::main + "\\" + _gdit + "\\" + "part_" + app::settings::sval("username");

        for (std::string com : methods::dall(methods::workdir() + "\\" + app::dir::main + "\\" + _gdit + "\\" + "part_" + app::settings::sval("username")))
            if (com.find(".commit.") != std::string::npos)
                remove((methods::workdir() + "\\" + app::dir::main + "\\" + _gdit + "\\" + "part_" + app::settings::sval("username") + "\\" + com).c_str());

        methods::fcopy(
            dir + "\\" + _gdit + ".og." + ext::level,
            dir + "\\" + _gdit + ".work." + ext::level
        );

        return GDIT_COMMIT_SUCCESS;
    }

    int MergeCommit(std::string _part_path) {
        std::string name = GetGDitNameFromCommit(_part_path);
        std::string dir = methods::workdir() + "\\" + app::dir::main + "\\" + name + "\\master";

        if (!methods::fexists(dir))
            return GDIT_MERGE_MASTER_DOESNT_EXIST;
        
        std::string base = methods::fread(dir + "\\" + name + ".master." + ext::leveldata);
        std::string commit = methods::fread(_part_path);

        std::string base_data;
        if (base._Starts_with("H4sIAAAA")) {
            base_data = gd::decode::DecodeLevelData(base);
        } else base_data = base;

        int s = commit.find_first_of(" ") + 1;
        if (std::stoi(commit.substr(s, commit.find_first_of("\n") - s - 1 /* /r */)) != GDIT_COMMIT_VERSION)
            return GDIT_MERGE_VERSION_NEWER;
        
        int b = commit.find("\nDATA ") + 6;
        nlohmann::json j = nlohmann::json::parse(commit.substr(commit.find_first_of("\n", b) + 1));

        for (std::string obj : j["removed"])
            base_data = methods::remove(base_data, obj + ";", false);
        
        std::string add = "";
        for (std::string obj : j["added"])
            add += methods::sanitize(obj) + ";";
        base_data += add;

        methods::fsave(dir + "\\" + name + ".master." + ext::leveldata, base_data);

        return GDIT_MERGE_SUCCESS;
    }

    std::string ViewGditLevel(std::string _gdit, bool _og = false) {
        gd::decode::GetCCLocalLevels();

        std::string dir = methods::workdir() + "\\" + app::dir::main + "\\" + _gdit + "\\master";
        if (_og) {
            std::string og = methods::fread(dir + "\\" + _gdit + ".master.og." + ext::level);
            gd::levels::SetKey(&og, "k2", "view_og@" + _gdit);
            gd::levels::ImportLevel_X("", og);
            return "Added to your GD levels under the name view_og@" + _gdit;
        }
        std::string base = methods::fread(dir + "\\" + _gdit + ".master." + ext::leveldata);
        nlohmann::json base_info = nlohmann::json::parse(methods::fread(dir + "\\" + _gdit + ".master." + ext::main));

        std::string lvl = "<d><k>kCEK</k><i>4</i><k>k4</k><s>" + base
        + "</s><k5>gdit</k5><k>k13</k><t /><k>k21</k><i>2</i>" + base_info["song"].dump() + "</d>";

        gd::levels::ImportLevel_X("", lvl, "view@" + _gdit);

        return "Added to your GD levels with the name view@" + _gdit;
    }
}