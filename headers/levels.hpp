#include <string>
#include <ShlObj.h>
#include <windows.h>
#include <vector>
#include <fstream>
#include <regex>
#include "main.hpp"
#include "../ext/ZlibHelper.hpp"
#include "../ext/Base64.hpp"
#include "../ext/json.hpp"
#include "../ext/dirent.h"

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
                app::decoded_data = c;
                return c;
            }

            DecodeXOR(CCCONTENTS, 11);
            auto XOR = std::string(CCCONTENTS.begin(), CCCONTENTS.end());
            std::vector<uint8_t> B64 = DecodeBase64(XOR);
            std::string ZLIB = DecompressGZip(B64);

            app::decoded_data = ZLIB;

            return ZLIB;
        }

        std::string GetCCLocalLevels() {
            if (app::decoded_data.empty())
                return DecodeCCLocalLevels();
            return app::decoded_data;
        }
    }

    namespace levels {
        void LoadLevels() {
            if (app::levels.size() == 0) {
                std::regex m ("<k>k_[0-9]+<\\/k>.*?<\\/d>.*?<\\/d>");
                std::smatch sm;

                std::string RED = decode::GetCCLocalLevels();
                std::vector<std::string> LIST (1);

                int i = 0;
                while (std::regex_search(RED, sm, m)) {
                    LIST.resize(i + 1);
                    LIST[i] = sm[0];

                    i++;

                    RED = RED.substr(sm[0].length() - 5);
                }

                app::levels = LIST;
            }
        }

        std::string GetKey(const std::string DATA, std::string KEY, std::string TYPE = ".*?") {
            if (TYPE == "") {
                std::regex m ("<k>" + KEY + "</k>");
                return (std::regex_search(DATA, m)) ? "True" : "False";
            } else {
                std::regex m ("<k>" + KEY + "</k><" + TYPE + ">");
                std::smatch cm;
                std::regex_search(DATA, cm, m);

                if (cm[0] == "") return "";

                std::string T_TYPE = ((std::string)cm[0]).substr(((std::string)cm[0]).find_last_of("<") + 1, 1);

                std::regex tm ("<k>" + KEY + "</k><" + T_TYPE + ">.*?</" + T_TYPE + ">");
                std::smatch tcm;
                std::regex_search(DATA, tcm, tm);

                std::string VAL = tcm[0];

                int L1 = ("<k>" + KEY + "</k><" + T_TYPE + ">").length();
                return VAL.substr(L1, VAL.find_last_of("</") - L1 - 1);
            }
        }

        std::string SetKey(std::string *_data, std::string _key, std::string _val) {
            int p = _data->find("<k>" + _key + "</k>") - 7 - _key.length();

            std::string m = _data->substr(_data->find("<k>" + _key + "</k>") + 7 + _key.length());
            std::string t = m.substr(1, 1);
            std::string r = "<k>" + _key + "</k><" + t + ">" + _val + "</" + t + ">";

            *_data = std::regex_replace(*_data, std::regex(R"(<k>)" + _key + R"(<\/k><.>.*?<\/.>)"), r, std::regex_constants::match_any);
            return r;
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

        std::string GetLevel(std::string _name, std::string *_err) {
            LoadLevels();

            std::string LVL = "";

            for (int i = 0; i < app::levels.size(); i++)
                if (methods::lower(GetKey(app::levels[i], "k2")) == methods::lower(_name))
                    LVL = app::levels[i];

            if (LVL == "")
                *_err = "Could not find level! (Replace spaces in name with _)";
            else return LVL;
            
            return "";
        }

        int ImportLevel(std::string _path, std::string _lvl = "", std::string _name = "") {
            std::string lvl;
            if (_lvl == "")
                lvl = methods::fread(_path);
            else lvl = _lvl;

            std::string data = decode::GetCCLocalLevels();

            data = std::regex_replace(data, std::regex (R"P(<k>k1<\/k><i>\d+?<\/i>)P"), "",
            std::regex_constants::match_any);
            std::string first_half = data.substr(0, data.find("<k>_isArr</k><t />"));
            std::string second_half = data.substr(data.find("<k>_isArr</k><t />") + 18);
            std::string work = second_half;
            std::smatch sm;
            while (std::regex_search(work, sm, std::regex ("<k>k_[0-9]+<\\/k>.*?<\\/d>.*?<\\/d>"))) {
                std::string m = sm[0];
                int i = std::stoi(m.substr(5, m.find("</") - 5)) + 1;
                std::string n = std::regex_replace((std::string)sm[0], std::regex("[0-9]+"), std::to_string(i),
                std::regex_constants::format_first_only);
                second_half = methods::replace(second_half, m, n);

                work = work.substr(sm[0].length());
            }
            if (_name != "")
                SetKey(&lvl, "k2", _name);
            data = first_half + "<k>_isArr</k><t /><k>k_0</k>" + lvl.substr(lvl.find("<d>")) + second_half;
            methods::fsave(decode::GetCCPath("LocalLevels"), data);

            return GDIT_IMPORT_SUCCESS;
        }
    }
}

namespace gdit {
    nlohmann::json GenerateGDitLevelInfo(std::string _data) {
        std::time_t t = std::time(0);
        std::tm now;
        localtime_s(&now, &t);
        std::stringstream ss;
        ss << (now.tm_year + 1900) << '-' << (now.tm_mon + 1) << '-' << now.tm_mday << '-' << now.tm_hour << '-' << now.tm_min;

        return {
            { "name", gd::levels::GetKey(_data, "k2") },
            { "init-time", ss.str() }
        };
    }

    int InitGdit(std::string _name, std::string _lvl, std::string *_rpath, bool nomaster = false) {
        std::string path = app::dir::main + "\\" + methods::lower(_name);
        if (_mkdir(path.c_str()) != 0)
            return GDIT_COULD_NOT_MAKE_DIR;
        else {
            std::string fpath = path + "\\" + "master\\" + methods::lower(_name) + "." + ext::master + ".";
            
            if (!nomaster)
                if (_mkdir((path + "\\" + "master").c_str()) != 0)
                    return GDIT_COULD_NOT_MAKE_DIR;
                else {
                    methods::fsave(fpath + ext::leveldata, gd::levels::GetKey(_lvl, "k4"));
                    methods::fsave(fpath + ext::levelinfo, gd::levels::WithoutKey(_lvl, "k4"));
                    methods::fsave(fpath + "og." + ext::level, _lvl);
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

    int AddGditPart(std::string _path, std::string _creator) {
        std::string name = GetGDitNameFromPath(_path);
        std::string tp = "";
        std::string rpath;
        std::string gdname = name + "@" + _creator;

        if (GditExists(name))
            tp = app::dir::main + "\\" + name;
        else InitGdit(name, methods::fread(_path), &rpath, true);
        
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
        gd::levels::ImportLevel(_path, "", gdname);
        return GDIT_IMPORT_SUCCESS;
    }

    void CommitChanges(std::string _gdit) {
        std::cout << _gdit << std::endl;
    }
}