#include "systray.hpp"
#include "nwm.hpp"
#include "bar.hpp"
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <iostream>
#include <algorithm>
#include <cstring>

#define TRAY_ICON_SIZE 20
#define TRAY_PADDING 4

void nwm::systray_send_message(Display *display, Window window, long message,
                               long data1, long data2, long data3) {
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = XInternAtom(display, "_XEMBED", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = CurrentTime;
    ev.xclient.data.l[1] = message;
    ev.xclient.data.l[2] = data1;
    ev.xclient.data.l[3] = data2;
    ev.xclient.data.l[4] = data3;
    XSendEvent(display, window, False, NoEventMask, &ev);
    XSync(display, False);
}

void nwm::systray_init(Base &base) {
    base.systray.icon_size = TRAY_ICON_SIZE;
    base.systray.padding = TRAY_PADDING;
    
    char tray_atom_name[32];
    snprintf(tray_atom_name, sizeof(tray_atom_name), "_NET_SYSTEM_TRAY_S%d", base.screen);
    base.systray.selection_atom = XInternAtom(base.display, tray_atom_name, False);
    base.systray.opcode_atom = XInternAtom(base.display, "_NET_SYSTEM_TRAY_OPCODE", False);
    base.systray.xembed_atom = XInternAtom(base.display, "_XEMBED", False);
    base.systray.xembed_info_atom = XInternAtom(base.display, "_XEMBED_INFO", False);
    
    Window existing_tray = XGetSelectionOwner(base.display, base.systray.selection_atom);
    if (existing_tray != None) {
        std::cerr << "Warning: Another system tray is already running" << std::endl;
        return;
    }
    
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.event_mask = SubstructureNotifyMask | SubstructureRedirectMask;
    attrs.background_pixel = 0x1A1A1A;
    attrs.border_pixel = 0x1A1A1A;
    
    base.systray.window = XCreateWindow(
        base.display, base.root,
        0, 0, 1, base.bar.height,
        0,
        DefaultDepth(base.display, base.screen),
        CopyFromParent,
        DefaultVisual(base.display, base.screen),
        CWOverrideRedirect | CWEventMask | CWBackPixel | CWBorderPixel,
        &attrs
    );
    
    XSetSelectionOwner(base.display, base.systray.selection_atom, 
                      base.systray.window, CurrentTime);
    
    if (XGetSelectionOwner(base.display, base.systray.selection_atom) != base.systray.window) {
        std::cerr << "Error: Failed to acquire system tray selection" << std::endl;
        XDestroyWindow(base.display, base.systray.window);
        base.systray.window = None;
        return;
    }
    
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.message_type = XInternAtom(base.display, "MANAGER", False);
    ev.xclient.display = base.display;
    ev.xclient.window = base.root;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = CurrentTime;
    ev.xclient.data.l[1] = base.systray.selection_atom;
    ev.xclient.data.l[2] = base.systray.window;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;
    
    XSendEvent(base.display, base.root, False, StructureNotifyMask, &ev);
    
    Atom orientation_atom = XInternAtom(base.display, "_NET_SYSTEM_TRAY_ORIENTATION", False);
    long orientation = 0;  // 0 = horizontal
    XChangeProperty(base.display, base.systray.window, orientation_atom,
                   XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&orientation, 1);
    
    Atom visual_atom = XInternAtom(base.display, "_NET_SYSTEM_TRAY_VISUAL", False);
    XVisualInfo template_info;
    template_info.screen = base.screen;
    template_info.depth = 32;
    template_info.c_class = TrueColor;
    
    int num_visuals;
    XVisualInfo *visual_info = XGetVisualInfo(base.display, 
                                             VisualScreenMask | VisualDepthMask | VisualClassMask,
                                             &template_info, &num_visuals);
    
    if (visual_info && num_visuals > 0) {
        XChangeProperty(base.display, base.systray.window, visual_atom,
                       XA_VISUALID, 32, PropModeReplace,
                       (unsigned char *)&visual_info[0].visualid, 1);
        XFree(visual_info);
    }
    
    XMapWindow(base.display, base.systray.window);
    XSync(base.display, False);
    
    std::cout << "System tray initialized successfully" << std::endl;
}

void nwm::systray_cleanup(Base &base) {
    for (auto &icon : base.systray.icons) {
        XUnmapWindow(base.display, icon.window);
        XReparentWindow(base.display, icon.window, base.root, 0, 0);
    }
    base.systray.icons.clear();
    
    if (base.systray.window != None) {
        XSetSelectionOwner(base.display, base.systray.selection_atom, None, CurrentTime);
        XDestroyWindow(base.display, base.systray.window);
        base.systray.window = None;
    }
}

int nwm::systray_get_width(Base &base) {
    if (base.systray.icons.empty()) return 0;
    
    int width = 0;
    for (const auto &icon : base.systray.icons) {
        if (icon.mapped) {
            width += base.systray.icon_size + base.systray.padding;
        }
    }
    return width + base.systray.padding;
}

void nwm::systray_update(Base &base) {
    int x_offset = 0;
    int tray_y = (base.bar.height - base.systray.icon_size) / 2;
    
    for (auto &icon : base.systray.icons) {
        if (!icon.mapped) continue;
        
        icon.x = x_offset + base.systray.padding;
        icon.y = tray_y;
        icon.width = base.systray.icon_size;
        icon.height = base.systray.icon_size;
        
        XMoveResizeWindow(base.display, icon.window,
                         icon.x, icon.y,
                         icon.width, icon.height);
        
        x_offset += base.systray.icon_size + base.systray.padding;
    }
    
    int total_width = x_offset + base.systray.padding;
    if (total_width > 0) {
        int bar_width = WIDTH(base.display, base.screen);
        int tray_x = bar_width - total_width - 12;
        
        int tray_window_y = base.bar_position == 0 ? 0 : HEIGHT(base.display, base.screen) - base.bar.height;
        
        XMoveResizeWindow(base.display, base.systray.window,
                         tray_x, tray_window_y,
                         total_width, base.bar.height);
        
        XSetWindowBackground(base.display, base.systray.window, 0x1A1A1A);
        XClearWindow(base.display, base.systray.window);
        XRaiseWindow(base.display, base.systray.window);
    }
    
    bar_draw(base);
}

void nwm::systray_add_icon(Base &base, Window icon) {
    for (const auto &existing : base.systray.icons) {
        if (existing.window == icon) {
            return;
        }
    }
    
    XSelectInput(base.display, icon, StructureNotifyMask | PropertyChangeMask);
    
    XAddToSaveSet(base.display, icon);
    XReparentWindow(base.display, icon, base.systray.window, 0, 0);
    
    TrayIcon tray_icon;
    tray_icon.window = icon;
    tray_icon.x = 0;
    tray_icon.y = 0;
    tray_icon.width = base.systray.icon_size;
    tray_icon.height = base.systray.icon_size;
    tray_icon.mapped = false;
    
    base.systray.icons.push_back(tray_icon);
    
    systray_send_message(base.display, icon, XEMBED_EMBEDDED_NOTIFY, 
                        0, base.systray.window, 0);
    
    XMapRaised(base.display, icon);
    
    for (auto &ic : base.systray.icons) {
        if (ic.window == icon) {
            ic.mapped = true;
            break;
        }
    }
    
    systray_update(base);
    
    std::cout << "System tray icon added: 0x" << std::hex << icon << std::dec << std::endl;
}

void nwm::systray_remove_icon(Base &base, Window icon) {
    auto it = std::find_if(base.systray.icons.begin(), base.systray.icons.end(),
                          [icon](const TrayIcon &ti) { return ti.window == icon; });
    
    if (it != base.systray.icons.end()) {
        XSelectInput(base.display, icon, NoEventMask);
        XUnmapWindow(base.display, icon);
        XReparentWindow(base.display, icon, base.root, 0, 0);
        
        base.systray.icons.erase(it);
        
        systray_update(base);
        
        std::cout << "System tray icon removed: 0x" << std::hex << icon << std::dec << std::endl;
    }
}

void nwm::systray_handle_client_message(Base &base, XClientMessageEvent *e) {
    if (e->message_type == base.systray.opcode_atom) {
        if (e->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
            Window icon = e->data.l[2];
            if (icon) {
                systray_add_icon(base, icon);
            }
        }
    }
}

void nwm::systray_handle_destroy(Base &base, Window window) {
    systray_remove_icon(base, window);
}

void nwm::systray_handle_configure_request(Base &base, XConfigureRequestEvent *e) {
    for (const auto &icon : base.systray.icons) {
        if (icon.window == e->window) {
            XWindowChanges wc;
            wc.x = icon.x;
            wc.y = icon.y;
            wc.width = icon.width;
            wc.height = icon.height;
            XConfigureWindow(base.display, e->window, 
                           CWX | CWY | CWWidth | CWHeight, &wc);
            return;
        }
    }
}
