#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <dwmapi.h>
#include <thread>
#include "../SDL/SDL.h"
#include "../SDL/SDL_syswm.h"
#include "style.h"
#include "../headers/errors.hpp"
#include "../SDL/SDL_ttf.h"

namespace gui {
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

        void text(SDL_Renderer* rx, std::string _txt, point p) {
            TTF_Font* Sans = TTF_OpenFont("resources/OpenSans.ttf", 24);

            SDL_Color White = {255, 255, 255}; 

            SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, _txt.c_str(), White);

            SDL_Texture* Message = SDL_CreateTextureFromSurface(rx, surfaceMessage);

            SDL_Rect Message_rect;
            Message_rect.x = p.x;
            Message_rect.y = p.y;
            Message_rect.w = 100;
            Message_rect.h = 100;

            SDL_RenderCopy(rx, Message, NULL, &Message_rect);

            std::cout << _txt;

            SDL_FreeSurface(surfaceMessage);
            SDL_DestroyTexture(Message);
        }
    }

    namespace sett {
        namespace app_size {
            draw::size size = { 320, 400 };
            int c = 8;
        }

        bool quit = false;

        void exit() {
            quit = true;
            SDL_Quit();
        }
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
            bool update = true;
            unsigned int id;
            std::vector<gdit_element> elements;

            void render();
            void init(std::string, draw::size);
            void handle(SDL_Event);
            void close();
    };

    namespace windows {
        unsigned int ids = 0;
        std::vector<gdit_window*> all = {};
    }

    void gdit_window::handle (SDL_Event _e) {
        if (SDL_GetWindowFlags(this->window) & SDL_WINDOW_INPUT_FOCUS)
            switch (_e.type) {
                case SDL_TEXTINPUT:
                    std::cout << _e.text.text;
                    break;
                case SDL_QUIT:
                    this->close();
                    break;
                case SDL_SYSWMEVENT:
                    if (_e.syswm.msg->msg.win.msg == WM_LBUTTONDOWN)
                        PostMessage(_e.syswm.msg->msg.win.hwnd, WM_SYSCOMMAND, SC_SIZE + 9, 0);
                    break;
                case SDL_KEYDOWN:
                    switch (_e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            this->close();
                            break;
                    }
                    break;
            }
    }

    void gdit_window::render() {
        SDL_SetRenderDrawColor(this->renderer,
            style::colors::chroma_key.R, style::colors::chroma_key.G, style::colors::chroma_key.B, 255);
        SDL_RenderClear(this->renderer);

        SDL_SetRenderDrawColor(this->renderer,
            style::colors::main_bg.R, style::colors::main_bg.G, style::colors::main_bg.B, 255);
        draw::rect(this->renderer, { 0, 0 }, { this->size.w, this->size.h }, sett::app_size::c);

        draw::text(this->renderer, this->name, { 0, 0 });

        SDL_RenderPresent(this->renderer);

        this->update = false;
    }

    void gdit_window::init (std::string _name, draw::size _sz = sett::app_size::size) {
        this->name = _name;
        this->id = windows::ids++;
        this->size = _sz;

        this->window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->size.w, this->size.h, SDL_WINDOW_BORDERLESS);
        this->renderer = SDL_CreateRenderer(this->window, 0, SDL_RENDERER_ACCELERATED);

        SDL_SetRenderDrawColor(this->renderer,
            style::colors::chroma_key.R, style::colors::chroma_key.G, style::colors::chroma_key.B, 255);
        SDL_RenderClear(this->renderer);

        //SDL_SetWindowOpacity(window, .5);
        gui::MakeWindowTransparent(this->window, style::colors::chroma_key_ref);
        SDL_RenderPresent(this->renderer);

        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(this->window, &wmInfo);
        this->hwnd = wmInfo.info.win.window;

        MARGINS margins = { 1, 0, 0, 0 };

        DwmExtendFrameIntoClientArea(this->hwnd, &margins);

        windows::all.push_back(this);
    }

    void gdit_window::close () {
        SDL_DestroyWindow(this->window);
        SDL_DestroyRenderer(this->renderer);

        if (this->id == 0)
            sett::exit();
    }
}