#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <dwmapi.h>
#include <thread>
#include "../SDL/SDL.h"
#include "../SDL/SDL_syswm.h"
#include "style.h"

namespace gui {
    namespace sett {
        namespace app_size {
            int w = 320;
            int h = 400;
            int c = 8;
        }

        bool quit = false;
    }

    namespace draw {
        struct point {
            int x; int y;
        };
        struct size {
            int w; int h;
        };

        void circle(SDL_Renderer* rx, point p, int r) {
            for (int w = 0; w < r * 2; w++) {
                for (int h = 0; h < r * 2; h++) {
                    int dx = r - w; // horizontal offset
                    int dy = r - h; // vertical offset
                    if ((dx*dx + dy*dy) <= (r * r))
                    {
                        SDL_RenderDrawPoint(rx, p.x + dx, p.y + dy);
                    }
                }
            }
        }

        void rect(SDL_Renderer* rx, point p, size s, int cr = 0) {
            SDL_Rect rect;
            if (cr <= 0) {
                rect.x = p.x; rect.y = p.y; rect.w = s.w; rect.h = s.h;
                SDL_RenderFillRect(rx, &rect);
            } else {
                rect.x = p.x + cr; rect.y = p.y + cr; rect.w = s.w - cr * 2; rect.h = s.h - cr * 2;
                SDL_Rect ns;
                ns.x = rect.x; ns.y = p.y; ns.w = rect.w; ns.h = s.h;
                SDL_Rect ew;
                ew.x = p.x; ew.y = rect.y; ew.w = s.w; ew.h = rect.h;
                SDL_RenderFillRect(rx, &ns);
                SDL_RenderFillRect(rx, &ew);
                circle(rx, { rect.x, rect.y }, cr);
                circle(rx, { rect.x + rect.w, rect.y }, cr);
                circle(rx, { rect.x, rect.y + rect.h }, cr);
                circle(rx, { rect.x + rect.w, rect.y + rect.h }, cr);
            }
        }
    }

    namespace windows {
        unsigned int amount = 0;
    }
    
    struct gdit_element {
        draw::point pos;
        draw::size size;
        std::string text;
        unsigned int id;
    };

    bool MakeWindowTransparent(SDL_Window* window, COLORREF colorKey) {
        // Get window handle (https://stackoverflow.com/a/24118145/3357935)
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);  // Initialize wmInfo
        SDL_GetWindowWMInfo(window, &wmInfo);
        HWND hWnd = wmInfo.info.win.window;

        // Change window type to layered (https://stackoverflow.com/a/3970218/3357935)
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        // Set transparency color
        return SetLayeredWindowAttributes(hWnd, colorKey, 0, LWA_COLORKEY);
    }

    class gdit_window {
        public:
            std::string name;
            SDL_Window* window;
            SDL_Renderer* renderer;
            draw::size size;
            HWND hwnd = NULL;
            bool quit = false;
            bool update = false;
            unsigned int id;
            std::vector<gdit_element> elements;
            gdit_window(std::string);
            ~gdit_window();
    };

    void run (gdit_window* wind) {
        SDL_Event event;

        while (!wind->quit) {
            if (wind->update) {
                SDL_SetRenderDrawColor(wind->renderer,
                    style::colors::chroma_key.R, style::colors::chroma_key.G, style::colors::chroma_key.B, 255);
                SDL_RenderClear(wind->renderer);
                SDL_SetRenderDrawColor(wind->renderer,
                    style::colors::main_bg.R, style::colors::main_bg.G, style::colors::main_bg.B, 255);
                draw::rect(wind->renderer, { 0, 0 }, { wind->size.w, wind->size.h }, sett::app_size::c);
                SDL_RenderPresent(wind->renderer);
                wind->update = false;
            }

            if (SDL_PollEvent(&event))
                switch (event.type) {
                    case SDL_TEXTINPUT:
                        std::cout << event.text.text;
                        break;
                    case SDL_QUIT:
                        wind->quit = true;
                        break;
                    case SDL_SYSWMEVENT:
                        if (event.syswm.msg->msg.win.msg == WM_LBUTTONDOWN)
                            PostMessage(event.syswm.msg->msg.win.hwnd, WM_SYSCOMMAND, SC_SIZE+9, 0);
                        break;
                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_ESCAPE:
                                wind->quit = true;
                                break;
                        }
                        break;
                }
        };
    }

    gdit_window::gdit_window (std::string _name) {
        name = _name;
        id = windows::amount++;
        int SDL_Init(SDL_INIT_VIDEO);
        SDL_VideoInit(NULL);

        window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gui::sett::app_size::w, gui::sett::app_size::h, SDL_WINDOW_BORDERLESS);
        renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

        SDL_SetRenderDrawColor(renderer,
            style::colors::chroma_key.R, style::colors::chroma_key.G, style::colors::chroma_key.B, 255);
        SDL_RenderClear(renderer);

        //SDL_SetWindowOpacity(window, .5);
        gui::MakeWindowTransparent(window, style::colors::chroma_key_ref);
        SDL_RenderPresent(renderer);

        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        hwnd = wmInfo.info.win.window;

        MARGINS margins = { 1, 0, 0, 0 };

        DwmExtendFrameIntoClientArea(hwnd, &margins);

        run(this);
    }

    gdit_window::~gdit_window () {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);

        if (id == 0)
            SDL_Quit();
    }
}