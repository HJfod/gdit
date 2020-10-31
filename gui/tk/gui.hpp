#include "../../ext/cpptk.h"

namespace gui {
    void main(char *argv[]) {
        Tk::init(argv[0]);
        
        Tk::label(".l") -Tk::text("Hello C++/Tk!");
        Tk::pack(".l") -Tk::padx(20) -Tk::pady(6);
        
        Tk::runEventLoop();
    }
}