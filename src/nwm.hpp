#ifndef NWM_HPP
#define NWM_HPP

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <vector>

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
    std::vector<ManagedWindow> windows;
    ManagedWindow* focused_window;
    bool running;
    Cursor cursor;

    XftFont* xft_font;
    XftDraw* xft_draw;
};

// Window management functions
void manage_window(Window window, Base &base);
void unmanage_window(Window window, Base &base);
void focus_window(ManagedWindow* window, Base &base);
void move_window(ManagedWindow* window, int x, int y, Base &base);
void resize_window(ManagedWindow* window, int width, int height, Base &base);
void tile_windows(Base &base);
void close_window(void *arg, Base &base);
void focus_next(void *arg, Base &base);
void focus_prev(void *arg, Base &base);
void quit_wm(void *arg, Base &base);


// Event handlers
void handle_key_press(XKeyEvent *e, Base &base);
void handle_button_press(XButtonEvent *e, Base &base);
void handle_configure_request(XConfigureRequestEvent *e, Base &base);
void handle_map_request(XMapRequestEvent *e, Base &base);
void handle_unmap_notify(XUnmapEvent *e, Base &base);
void handle_enter_notify(XCrossingEvent *e, Base &base);
void handle_destroy_notify(XDestroyWindowEvent *e, Base &base);

void reload_config(void *arg, Base &base);
void spawn(void *arg, Base &base);

// System functions
void setup_keys(Base &base);
void run(Base &base);
void init(Base &base);
void cleanup(Base &base);

}

#endif // NWM_HPP
