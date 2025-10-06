#include "bar.hpp"
#include "nwm.hpp"
#include "util.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <X11/Xlib.h>

// TODO: Let users customize it here

#define BAR_HEIGHT 20
#define BAR_BG_COLOR 0x1a1a1a
#define BAR_FG_COLOR 0xcccccc
#define BAR_ACTIVE_COLOR 0xFF5577
#define BAR_INACTIVE_COLOR 0x666666

void nwm::bar_init(Base &base) {
    base.bar.height = BAR_HEIGHT;
    base.bar.width = WIDTH(base.display, base.screen);
    base.bar.x = 0;
    base.bar.y = 0;

    XSetWindowAttributes attrs;
    attrs.background_pixel = BAR_BG_COLOR;
    attrs.override_redirect = True;
    attrs.event_mask = ExposureMask | ButtonPressMask;

    base.bar.window = XCreateWindow(
        base.display, base.root,
        base.bar.x, base.bar.y,
        base.bar.width, base.bar.height,
        0,
        DefaultDepth(base.display, base.screen),
        CopyFromParent,
        DefaultVisual(base.display, base.screen),
        CWBackPixel | CWOverrideRedirect | CWEventMask,
        &attrs
    );

    XMapWindow(base.display, base.bar.window);
    XRaiseWindow(base.display, base.bar.window);

    base.bar.xft_draw = XftDrawCreate(
        base.display, base.bar.window,
        DefaultVisual(base.display, base.screen),
        DefaultColormap(base.display, base.screen)
    );

    XRenderColor xrender_fg = {
        (unsigned short)((BAR_FG_COLOR >> 16) & 0xFF) * 257,
        (unsigned short)((BAR_FG_COLOR >> 8) & 0xFF) * 257,
        (unsigned short)(BAR_FG_COLOR & 0xFF) * 257,
        65535
    };
    XftColorAllocValue(base.display, DefaultVisual(base.display, base.screen),
                       DefaultColormap(base.display, base.screen),
                       &xrender_fg, &base.bar.xft_fg);

    XRenderColor xrender_bg = {
        (unsigned short)((BAR_BG_COLOR >> 16) & 0xFF) * 257,
        (unsigned short)((BAR_BG_COLOR >> 8) & 0xFF) * 257,
        (unsigned short)(BAR_BG_COLOR & 0xFF) * 257,
        65535
    };
    XftColorAllocValue(base.display, DefaultVisual(base.display, base.screen),
                       DefaultColormap(base.display, base.screen),
                       &xrender_bg, &base.bar.xft_bg);

    XRenderColor xrender_active = {
        (unsigned short)((BAR_ACTIVE_COLOR >> 16) & 0xFF) * 257,
        (unsigned short)((BAR_ACTIVE_COLOR >> 8) & 0xFF) * 257,
        (unsigned short)(BAR_ACTIVE_COLOR & 0xFF) * 257,
        65535
    };
    XftColorAllocValue(base.display, DefaultVisual(base.display, base.screen),
                       DefaultColormap(base.display, base.screen),
                       &xrender_active, &base.bar.xft_active);

    XRenderColor xrender_inactive = {
        (unsigned short)((BAR_INACTIVE_COLOR >> 16) & 0xFF) * 257,
        (unsigned short)((BAR_INACTIVE_COLOR >> 8) & 0xFF) * 257,
        (unsigned short)(BAR_INACTIVE_COLOR & 0xFF) * 257,
        65535
    };
    XftColorAllocValue(base.display, DefaultVisual(base.display, base.screen),
                       DefaultColormap(base.display, base.screen),
                       &xrender_inactive, &base.bar.xft_inactive);
}

void nwm::bar_cleanup(Base &base) {
    if (base.bar.xft_draw) {
        XftDrawDestroy(base.bar.xft_draw);
        base.bar.xft_draw = nullptr;
    }

    XftColorFree(base.display, DefaultVisual(base.display, base.screen),
                 DefaultColormap(base.display, base.screen), &base.bar.xft_fg);
    XftColorFree(base.display, DefaultVisual(base.display, base.screen),
                 DefaultColormap(base.display, base.screen), &base.bar.xft_bg);
    XftColorFree(base.display, DefaultVisual(base.display, base.screen),
                 DefaultColormap(base.display, base.screen), &base.bar.xft_active);
    XftColorFree(base.display, DefaultVisual(base.display, base.screen),
                 DefaultColormap(base.display, base.screen), &base.bar.xft_inactive);

    if (base.bar.window) {
        XDestroyWindow(base.display, base.bar.window);
        base.bar.window = 0;
    }
}

void nwm::bar_draw(Base &base) {
    XClearWindow(base.display, base.bar.window);

    if (!base.xft_font || !base.bar.xft_draw) return;

    int x_offset = 10;
    int y_offset = BAR_HEIGHT / 2 + 5;

    // draw workspaces
    for (size_t i = 0; i < base.workspaces.size(); ++i) {
        std::string ws_label = "[" + std::to_string(i + 1) + "]";
        
        XftColor *color = (i == base.current_workspace) ? 
                          &base.bar.xft_active : &base.bar.xft_inactive;

        XGlyphInfo extents;
        XftTextExtentsUtf8(base.display, base.xft_font,
                          (XftChar8*)ws_label.c_str(), ws_label.length(),
                          &extents);

        XftDrawStringUtf8(base.bar.xft_draw, color, base.xft_font,
                         x_offset, y_offset,
                         (XftChar8*)ws_label.c_str(), ws_label.length());

        x_offset += extents.width + 15;
    }

    // draw layout mode indicator
    x_offset += 20;
    std::string layout_mode = base.horizontal_mode ? "[HORIZ]" : "[TILE]";
    XftDrawStringUtf8(base.bar.xft_draw, &base.bar.xft_fg, base.xft_font,
                     x_offset, y_offset,
                     (XftChar8*)layout_mode.c_str(), layout_mode.length());

    // draw time on the right
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    std::ostringstream time_stream;
    time_stream << std::put_time(local, "%H:%M");
    std::string time_str = time_stream.str();

    XGlyphInfo time_extents;
    XftTextExtentsUtf8(base.display, base.xft_font,
                      (XftChar8*)time_str.c_str(), time_str.length(),
                      &time_extents);

    int time_x = base.bar.width - time_extents.width - 10;
    XftDrawStringUtf8(base.bar.xft_draw, &base.bar.xft_fg, base.xft_font,
                     time_x, y_offset,
                     (XftChar8*)time_str.c_str(), time_str.length());

    XFlush(base.display);
}

void nwm::bar_update_workspaces(Base &base) {
    bar_draw(base);
}

void nwm::bar_update_time(Base &base) {
    bar_draw(base);
}
