#ifndef SYSTRAY_HPP
#define SYSTRAY_HPP

#include <X11/Xlib.h>
#include <vector>

namespace nwm {

struct Base;

#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2
#define XEMBED_REQUEST_FOCUS 3
#define XEMBED_FOCUS_IN 4
#define XEMBED_FOCUS_OUT 5
#define XEMBED_FOCUS_NEXT 6
#define XEMBED_FOCUS_PREV 7
#define XEMBED_MODALITY_ON 10
#define XEMBED_MODALITY_OFF 11
#define XEMBED_REGISTER_ACCELERATOR 12
#define XEMBED_UNREGISTER_ACCELERATOR 13
#define XEMBED_ACTIVATE_ACCELERATOR 14

#define XEMBED_FOCUS_CURRENT 0
#define XEMBED_FOCUS_FIRST 1
#define XEMBED_FOCUS_LAST 2

#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

struct TrayIcon {
    Window window;
    int x, y;
    int width, height;
    bool mapped;
};

struct SystemTray {
    Window window;
    Atom selection_atom;
    Atom opcode_atom;
    Atom xembed_atom;
    Atom xembed_info_atom;
    std::vector<TrayIcon> icons;
    int icon_size;
    int padding;
};

void systray_init(Base &base);
void systray_cleanup(Base &base);
void systray_update(Base &base);
void systray_add_icon(Base &base, Window icon);
void systray_remove_icon(Base &base, Window icon);
void systray_handle_client_message(Base &base, XClientMessageEvent *e);
void systray_handle_destroy(Base &base, Window window);
void systray_handle_configure_request(Base &base, XConfigureRequestEvent *e);
void systray_send_message(Display *display, Window window, long message,
                         long data1, long data2, long data3);
int systray_get_width(Base &base);

}

#endif
