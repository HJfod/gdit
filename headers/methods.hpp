#include <string>
#include <Windows.h>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <direct.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <conio.h>
#include "errors.hpp"
#include "../ext/dirent.h"
#include "../ext/rapidxml-1.13/rapidxml.hpp"
#include "../ext/rapidxml-1.13/rapidxml_print.hpp"

namespace methods {
    std::string replace(std::string const& original, std::string const& from, std::string const& to) {
        std::string results;
        std::string::const_iterator end = original.end();
        std::string::const_iterator current = original.begin();
        std::string::const_iterator next = std::search( current, end, from.begin(), from.end() );
        while ( next != end ) {
            results.append( current, next );
            results.append( to );
            current = next + from.size();
            next = std::search( current, end, from.begin(), from.end() );
        }
        results.append( current, next );
        return results;
    }

    std::string lower(std::string const& _s) {
        std::string s = _s;
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    bool fexists(std::string _path) {
        struct stat info;
        return !(stat(_path.c_str(), &info ) != 0);
    }

    std::string workdir() {
        char buff[FILENAME_MAX];
        _getcwd(buff, FILENAME_MAX);
        std::string current_working_dir(buff);
        return current_working_dir;
    }

    void fsave (std::string _path, std::string _cont) {
        std::ofstream file;
        file.open(_path);
        file << _cont;
        file.close();
    }

    std::string fread (std::string _path) {
        std::ifstream in(_path, std::ios::in | std::ios::binary);
        if (in) {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return(contents);
        } throw(errno);
    }

    int fcopy(std::string from, std::string to) {
        if (!fexists(from))
            return GDIT_COPY_FROM_DOESNT_EXIST;
        std::ifstream src(from, std::ios::binary);
        std::ofstream dst(to,   std::ios::binary);
        dst << src.rdbuf();

        return GDIT_COPY_SUCCESS;
    }

    bool ewith (std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    std::string sanitize (std::string _str) {
        std::string ret = _str;
        while (ret._Starts_with("\""))
            ret = ret.substr(1);
        while (ewith(ret, "\""))
            ret = ret.substr(0, ret.length() - 1);
        return ret;
    }

    std::vector<std::string> split (std::string _str, std::string _split) {
        size_t pos = 0;
        std::string token;
        std::vector<std::string> res = {};
        while ((pos = _str.find(_split)) != std::string::npos) {
            token = _str.substr(0, pos);
            res.push_back(token);
            _str.erase(0, pos + _split.length());
        }
        return res;
    }

    std::wstring conv (std::string _str) {
        return std::wstring(_str.begin(), _str.end());
    }

    std::vector<std::string> dread (std::string _path, bool (*_filter)(std::string, int), int _eparam = GDIT_NULL) {
        struct dirent *entry;
        DIR *dir = opendir(_path.c_str());

        std::vector<std::string> cont;
        while ((entry = readdir(dir)) != NULL)
            if (entry->d_type == DT_DIR)
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    if (_filter(_path + "\\" + entry->d_name, _eparam))
                        cont.push_back(entry->d_name);
        closedir(dir);

        return cont;
    }

    std::vector<std::string> dall (std::string _path) {
        struct dirent *entry;
        DIR *dir = opendir(_path.c_str());

        std::vector<std::string> cont;
        while ((entry = readdir(dir)) != NULL)
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                cont.push_back(entry->d_name);
        closedir(dir);

        return cont;
    }

    int count (std::string _s, char _c) {
        int count = 0;
        for (int i = 0; i < _s.size(); i++)
            if (_s[i] == _c) count++;
        return count;
    }

    std::string time (std::string _sep = "-") {
        std::time_t t = std::time(0);
        std::tm now;
        localtime_s(&now, &t);
        std::stringstream ss;
        ss << (now.tm_year + 1900) << _sep << (now.tm_mon + 1) << _sep << now.tm_mday << _sep << now.tm_hour << _sep << now.tm_min;

        return ss.str();
    }

    std::string remove(std::string _str, std::string _sub, bool all = false) {
        if (all) {
            int s;
            while ((s = _str.find(_sub)) != std::string::npos)
                _str = _str.erase(s, _sub.length());
            return _str;
        }
        int s = _str.find(_sub);
        if (s != std::string::npos)
            return _str.erase(s, _sub.length());
        else return _str;
    }

    char* stc (std::string _str) {
        int sz = _str.size() + 1;
        char* cstr = new char[sz];
        int err = strcpy_s(cstr, sz, _str.c_str());
        return cstr;
    }

    std::string xts (rapidxml::xml_node<>* _xml) {
        std::string res;
        rapidxml::print(std::back_inserter(res), *_xml, 0);
        return res;
    }

    std::string xts (rapidxml::xml_document<>* _xml) {
        std::string res;
        rapidxml::print(std::back_inserter(res), *_xml, 0);
        return res;
    }

    namespace perf {
        std::chrono::time_point<std::chrono::high_resolution_clock> perf_;

        void start() {
            perf_ = std::chrono::high_resolution_clock().now();
        }

        int end() {
            auto end_ = std::chrono::high_resolution_clock().now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - perf_).count();
        }

        void log(std::string _text) {
            std::cout << _text << "\t " << end() << "ms" << std::endl;
        }
    }

    struct range {
        int start;
        int end;
        bool contains(int _n) {
            return (_n >= start && _n <= end);
        }
    };

    struct range_col {
        void in();
    };

    struct range_super {
        // stored as 11-bit ints.
        // since fucking no one (except maybe akira kurisu) is going to use over 2000 colors.
        // i'm too lazy to figure out how to store 11-bit ints in c++ though, so let's just not bother.

        unsigned short start, end;
        range_col excluded;
        bool contains(unsigned short _n) {
            return (_n >= start && _n <= end) && (excluded.contains());
        }
    };

    struct range_col {
        void in() {
            
        }
    }
}

namespace console {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD CursorPosition;

    void gotoXY(int _x, int _y) {
        CursorPosition.X = _x;
        CursorPosition.Y = _y;
        SetConsoleCursorPosition(console, CursorPosition);
    };

    int selectmenu (std::vector<std::string> options, std::string *ret) {
        std::cout << "[Use arrow keys to navigate, space / enter to select]" << std::endl;

        for (std::string option : options)
            std::cout << " * " << option << std::endl;
        
        CONSOLE_SCREEN_BUFFER_INFO cbsi;
        GetConsoleScreenBufferInfo(console, &cbsi);
            
        bool selected = false;
        int last = 0;
        int selin = 0;
        while (!selected) {
            gotoXY(cbsi.dwCursorPosition.X, cbsi.dwCursorPosition.Y - options.size() + last);
            std::cout << " * ";
            gotoXY(cbsi.dwCursorPosition.X, cbsi.dwCursorPosition.Y - options.size() + selin);
            std::cout << " > ";
            last = selin;
            gotoXY(cbsi.dwCursorPosition.X, cbsi.dwCursorPosition.Y);

            switch (_getch()) {
                case '\r': case ' ':
                    selected = true;
                    break;
                case 27:
                    selected = true;
                    selin = -1;
                    break;
                case 0: case 224:
                    switch (_getch()) {
                        case 72: case 75:
                            selin--;
                            if (selin < 0) selin = options.size() - 1;
                            break;
                        case 80: case 77:
                            selin++;
                            if (selin > options.size() - 1) selin = 0;
                            break;
                    }
                    break;
            }
        }
        if (selin >= 0)
            *ret = options[selin];
        return selin;
    }

    void loadbar (std::string _txt, bool *_end) {
        std::chrono::milliseconds s = std::chrono::milliseconds(200);
        while (!*_end) {
            std::cout << "\r / "  << _txt;
            std::this_thread::sleep_for(s);
            std::cout << "\r - "  << _txt;
            std::this_thread::sleep_for(s);
            std::cout << "\r \\ " << _txt;
            std::this_thread::sleep_for(s);
            std::cout << "\r | "  << _txt;
            std::this_thread::sleep_for(s);
        }
        std::wcout << "\r* " << methods::conv(_txt) << std::endl;
    }

    std::thread showload (std::string txt, bool *end) {
        return std::thread(loadbar, txt, end);
    }
}