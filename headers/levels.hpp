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

    std::string GetGditLevel(std::string _name) {
        std::string to = methods::workdir() + "\\" + app::dir::copies + "\\" + _name + ".copy." + ext::level;
        methods::fcopy(
            app::dir::main + "\\" + _name + "\\master\\" + _name + ".master.og." + ext::level, to
        );
        return to;
    }
}