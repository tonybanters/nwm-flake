#ifndef NWM_HPP
#define NWM_HPP

/* Include stuff */
#include "api.hpp"
#include <X11/Xlib.h>
#include <fstream>
#include <vector>
#include <map>

#define BORDER_WIDTH 2
#define SCREEN_NUMBER 0
#define POSITION_X 0
#define POSITION_Y 0

#define WIDTH(display, screen_number) XDisplayWidth((display), (screen_number))
#define HEIGHT(display, screen_number) XDisplayHeight((display), (screen_number))

namespace nwm {
struct ManagedWindow {
    Window window;
    int x, y;
    int width, height;
    bool is_floating;
    bool is_focused;
};

struct Base {
    int screen;
    Window root;
    Display *display;
    std::fstream config;
    std::string error;
    std::vector<ManagedWindow> windows;
    ManagedWindow* focused_window;
};

struct De {
    Window window;
    XEvent *event;
};

// Window management functions
void manage_window(Window window, Base &base);
void unmanage_window(Window window, Base &base);
void focus_window(ManagedWindow* window, Base &base);
void move_window(ManagedWindow* window, int x, int y, Base &base);
void resize_window(ManagedWindow* window, int width, int height, Base &base);
void tile_windows(Base &base);

// Event handlers
void handle_key_press(XKeyEvent *, Base &, application::app &);
void handle_button_press(XButtonEvent *, Base &, application::app &);
void handle_configure_request(XConfigureRequestEvent *, Base &);
void handle_map_request(XMapRequestEvent *, Base &, application::app &);
void handle_unmap_notify(XUnmapEvent *, Base &);
void handle_destroy_notify(XDestroyWindowEvent *, Base &);

// Start stuff
void update(De &, Base &, application::app &);
void init(Base &);
void clean(Base &, De &);
void file_check(Base &);

} // namespace nwm

#endif // NWM_HPP
