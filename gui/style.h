#include <Windows.h>

namespace style {
    struct C_RGB {
        unsigned short R = 0;
        unsigned short G = 0;
        unsigned short B = 0;
    };

    unsigned int wind_corner_size = 10;
    unsigned int option_height = 30;

    namespace colors {
        C_RGB chroma_key = { 0, 255, 0 };
        COLORREF chroma_key_ref = RGB(chroma_key.R, chroma_key.G, chroma_key.B);
        C_RGB main_bg = { 107, 85, 71 };
        C_RGB main_text = { 0, 0, 0 };
    }
}