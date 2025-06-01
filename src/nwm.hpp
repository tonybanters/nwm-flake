#ifndef NWM_HPP
#define NWM_HPP

/* Include stuff */
#include <X11/Xlib.h>
#include <fstream>
#include <filesystem>
#include <memory>

#define BORDER 0 
#define SCREEN_NUMBER 0 
#define POSITION_X 0
#define POSITION_Y 0
#define FILE "~/.config/nwm/nwm.lua"
#define WIDTH(display, screen_number) XDisplayWidth((display), (screen_number))
#define HEIGHT(display, screen_number) XDisplayHeight((display), (screen_number))
namespace nwm {
    struct XDisplayDeleter {
        void operator()(Display* dpy) const {
            if (dpy) {
                XCloseDisplay(dpy);
            }
        }
    };

    using UniqueDisplay = std::unique_ptr<Display, XDisplayDeleter>;

    struct Base {
        int screen;
        UniqueDisplay display;
        Window root;
        std::fstream config;
        Display* getDisplay() const;
        void setDisplay(Display* dpy);
        std::string error;
    };

    struct De {
        Window window;
        XEvent event;
    };
}

void update(Window, XEvent, nwm::Base&);
void init(nwm::Base&);
void clean(nwm::Base&, Window);

#endif // NWM_HPP
