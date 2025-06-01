// THis is an x11 based window manager for me to learn x11 protocol
#ifndef NWM_HPP
#define NWM_HPP

//MACROS
#include <X11/Xlib.h>
#define WIDTH 40
#define BORDER 0 
#define HEIGHT 40
#define POSITION_X 20
#define POSITION_Y 30

namespace nwm {
struct UI_elt {
    int screen;
    Display* display;
    Window root;
};
struct De{
    Window window;
    XEvent event;
};
}
void update(Window, XEvent,nwm::UI_elt&);
void init(nwm::UI_elt&);
void clean(nwm::UI_elt&, Window);
#endif //NWM_HPP
