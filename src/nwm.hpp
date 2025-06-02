#ifndef NWM_HPP
#define NWM_HPP

/* Include stuff */
#include <X11/Xlib.h>
#include <fstream>

#define BORDER 0 
#define SCREEN_NUMBER 0 
#define POSITION_X 0
#define POSITION_Y 0
#define FILE "~/.config/nwm/nwm.lua"
#define WIDTH(display, screen_number) XDisplayWidth((display), (screen_number))
#define HEIGHT(display, screen_number) XDisplayHeight((display), (screen_number))
namespace nwm {
    struct Base {
        int screen;
        Window root;
        Display *display;
        std::fstream config;
        std::string error;
    };
    struct De {
        Window window;
        XEvent *event;
    };
}

void update(nwm::De&, nwm::Base&);
void init(nwm::Base&);
void clean(nwm::Base&, nwm::De&);

#endif // NWM_HPP
