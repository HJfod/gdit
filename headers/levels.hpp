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

        int ImportLevel(std::string _path) {
            std::string data = decode::GetCCLocalLevels();

            data = std::regex_replace(data, std::regex (R"P(<k>k1<\/k><i>\d+?<\/i>)P"), "",
            std::regex_constants::match_any);
            std::vector<std::string> split = methods::split(data, "<k>_isArr</k><t />");
            split[1] = std::regex_replace(split[1], std::regex (R"P(<k>k_(\d+)<\/k><d><k>kCEK<\/k>)P"), 
            );

            /*
            data = Regex.Replace(data, @"<k>k1<\/k><i>\d+?<\/i>", "");
            string[] splitData = data.Split("<k>_isArr</k><t />");
            splitData[1] = Regex.Replace(splitData[1], @"<k>k_(\d+)<\/k><d><k>kCEK<\/k>",
            m => $"<k>k_{(Int32.Parse((Regex.Match(m.Value, @"k_\d+").Value.Substring(2))) + 1)}</k><d><k>kCEK</k>");
            data = splitData[0] + "<k>_isArr</k><t /><k>k_0</k>" + lvl + splitData[1];
            */
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

    std::vector<std::string> GetAllRepos() {
        struct dirent *entry;
        DIR *dir = opendir(app::dir::main.c_str());

        std::vector<std::string> cont;
        while ((entry = readdir(dir)) != NULL)
            if (entry->d_type == DT_DIR)
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    if (methods::fexists(app::dir::main + "\\" + entry->d_name + "\\master"))
                        cont.push_back(entry->d_name);

        closedir(dir);

        return cont;
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
        std::string gdname = "part@" + name;

        if (GditExists(name))
            tp = app::dir::main + "\\" + name;
        else InitGdit(name, methods::fread(_path), &rpath, true);
        
        tp = tp == "" ? rpath : tp;

        if (_mkdir((tp + "\\" + "\\part_" + _creator).c_str()) != 0)
            return GDIT_COULD_NOT_MAKE_DIR;
        else {
            methods::fcopy(
                _path, tp + "\\part_" + _creator + "\\" + name + ".og." + ext::level
            );
            methods::fcopy(
                _path, tp + "\\part_" + _creator + "\\" + name + ".work." + ext::level
            );
            // TODO: actually import the part in gd
            return GDIT_IMPORT_SUCCESS;
        }
    }
}