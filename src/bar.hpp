#ifndef BAR_HPP
#define BAR_HPP

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <string>
#include <vector>

namespace nwm {

struct Base;

struct StatusBar {
    Window window;
    int x, y;
    int width, height;
    XftDraw* xft_draw;
    XftColor xft_fg;
    XftColor xft_bg;
    XftColor xft_active;
    XftColor xft_inactive;
};

void bar_init(Base &base);
void bar_cleanup(Base &base);
void bar_draw(Base &base);
void bar_update_workspaces(Base &base);
void bar_update_time(Base &base);

}

#endif // BAR_HPP
