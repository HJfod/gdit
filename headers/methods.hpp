#include <string>
#include <Windows.h>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <direct.h>
#include <fstream>
#include <conio.h>
#include "errors.hpp"

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
}