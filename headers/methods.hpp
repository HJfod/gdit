#include <string>
#include <Windows.h>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <direct.h>
#include <fstream>

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
}