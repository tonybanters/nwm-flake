#include "nwm.hpp"
#include "config.hpp"
#include "bar.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <cmath>
#include <cstring>

int x_error_handler(Display *dpy, XErrorEvent *error) {
    char error_text[1024];
    XGetErrorText(dpy, error->error_code, error_text, sizeof(error_text));
    std::cerr << "X Error: " << error_text
              << " Request code: " << (int)error->request_code
              << " Resource ID: " << error->resourceid << std::endl;
    return 0;
}

bool should_float(Display *display, Window window) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = nullptr;
    
    Atom window_type_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    if (XGetWindowProperty(display, window, window_type_atom, 0, 1,
                          False, XA_ATOM, &actual_type, &actual_format,
                          &nitems, &bytes_after, &prop) == Success && prop) {
        Atom type = *(Atom*)prop;
        XFree(prop);
        
        Atom dialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
        Atom splash = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
        Atom utility = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
        Atom toolbar = XInternAtom(display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
        Atom menu = XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", False);
        Atom dropdown = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False);
        Atom popup = XInternAtom(display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
        Atom notification = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
        
        if (type == dialog || type == splash || type == utility || 
            type == toolbar || type == menu || type == dropdown || 
            type == popup || type == notification) {
            return true;
        }
    }
    
    Atom state_atom = XInternAtom(display, "_NET_WM_STATE", False);
    if (XGetWindowProperty(display, window, state_atom, 0, 32,
                          False, XA_ATOM, &actual_type, &actual_format,
                          &nitems, &bytes_after, &prop) == Success && prop) {
        Atom *states = (Atom*)prop;
        Atom modal = XInternAtom(display, "_NET_WM_STATE_MODAL", False);
        
        for (unsigned long i = 0; i < nitems; i++) {
            if (states[i] == modal) {
                XFree(prop);
                return true;
            }
        }
        XFree(prop);
    }
    
    Window transient_for;
    if (XGetTransientForHint(display, window, &transient_for)) {
        return true;
    }
    
    XWindowAttributes attr;
    if (XGetWindowAttributes(display, window, &attr)) {
        if (attr.override_redirect) {
            return true;
        }
    }
    
    XSizeHints hints;
    long supplied;
    if (XGetWMNormalHints(display, window, &hints, &supplied)) {
        if ((hints.flags & PMaxSize) && (hints.flags & PMinSize)) {
            if (hints.max_width == hints.min_width && 
                hints.max_height == hints.min_height) {
                return true;
            }
        }
    }
    
    return false;
}

void nwm::workspace_init(Base &base) {
    base.workspaces.resize(NUM_WORKSPACES);
    for (auto &ws : base.workspaces) {
        ws.focused_window = nullptr;
        ws.scroll_offset = 0;
    }
    base.current_workspace = 0;
    base.overview_mode = false;
    base.dragging = false;
    base.drag_window = None;
    base.drag_start_x = 0;
    base.drag_start_y = 0;
}

nwm::Workspace& nwm::get_current_workspace(Base &base) {
    return base.workspaces[base.current_workspace];
}

void nwm::switch_workspace(void *arg, Base &base) {
    if (!arg) return;
    
    int target_ws = *(int*)arg;
    if (target_ws < 0 || target_ws >= NUM_WORKSPACES) return;
    if (target_ws == (int)base.current_workspace) return;

    for (auto &w : get_current_workspace(base).windows) {
        if (!w.is_floating) {
            XUnmapWindow(base.display, w.window);
        }
    }

    base.current_workspace = target_ws;
    base.focused_window = get_current_workspace(base).focused_window;

    for (auto &w : get_current_workspace(base).windows) {
        XMapWindow(base.display, w.window);
        if (w.is_floating) {
            XRaiseWindow(base.display, w.window);
        }
    }

    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }

    if (base.focused_window) {
        focus_window(base.focused_window, base);
    }

    bar_update_workspaces(base);
}

void nwm::move_to_workspace(void *arg, Base &base) {
    if (!arg || !base.focused_window) return;

    int target_ws = *(int*)arg;
    if (target_ws < 0 || target_ws >= NUM_WORKSPACES) return;

    auto &current_ws = get_current_workspace(base);
    
    for (auto it = current_ws.windows.begin(); it != current_ws.windows.end(); ++it) {
        if (it->window == base.focused_window->window) {
            ManagedWindow w = *it;
            w.workspace = target_ws;
            current_ws.windows.erase(it);
            
            base.workspaces[target_ws].windows.push_back(w);
            
            XUnmapWindow(base.display, w.window);
            
            if (current_ws.focused_window && current_ws.focused_window->window == w.window) {
                current_ws.focused_window = nullptr;
            }
            base.focused_window = nullptr;
            
            if (!current_ws.windows.empty()) {
                focus_window(&current_ws.windows[0], base);
            }
            
            break;
        }
    }

    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
}

void nwm::setup_keys(nwm::Base &base) {
    XUngrabKey(base.display, AnyKey, AnyModifier, base.root);

    for (auto &k : keys) {
        KeyCode code = XKeysymToKeycode(base.display, k.keysym);
        if (!code) continue;

        unsigned int modifiers[] = {0, LockMask, Mod2Mask, Mod2Mask | LockMask};
        for (unsigned int mod : modifiers) {
            XGrabKey(
                base.display,
                code,
                k.mod | mod,
                base.root,
                False,
                GrabModeAsync,
                GrabModeAsync
            );
        }
    }

    XGrabButton(base.display, Button1, MODKEY, base.root, False,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(base.display, Button3, MODKEY, base.root, False,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(base.display, Button4, MODKEY, base.root, False,
                ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(base.display, Button5, MODKEY, base.root, False,
                ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    XSync(base.display, False);
}

void nwm::spawn(void *arg, nwm::Base &base) {
    const char **cmd = (const char **)arg;
    if (fork() == 0) {
        setsid();
        execvp(cmd[0], (char **)cmd);
        perror("execvp failed");
        exit(1);
    }

    XSetInputFocus(base.display, base.root, RevertToPointerRoot, CurrentTime);
}

void nwm::toggle_gap(void *arg, nwm::Base &base) {
    (void)arg;
    base.gaps_enabled = !base.gaps_enabled;
    base.gaps = base.gaps_enabled ? GAP_SIZE : 0;
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
}

void nwm::toggle_bar(void *arg, Base &base) {
    (void)arg;
    base.bar_visible = !base.bar_visible;
    
    if (base.bar_visible) {
        XMapWindow(base.display, base.bar.window);
    } else {
        XUnmapWindow(base.display, base.bar.window);
    }
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    
    XFlush(base.display);
}

void nwm::toggle_layout(void *arg, Base &base) {
    (void)arg;
    base.horizontal_mode = !base.horizontal_mode;
    
    auto &current_ws = get_current_workspace(base);
    current_ws.scroll_offset = 0;

    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    
    for (auto &w : current_ws.windows) {
        if (w.is_floating) {
            XRaiseWindow(base.display, w.window);
        }
    }
    
    if (base.focused_window) {
        XRaiseWindow(base.display, base.focused_window->window);
    }
    
    XFlush(base.display);
    bar_draw(base);
}

void nwm::scroll_left(void *arg, Base &base) {
    (void)arg;
    if (!base.horizontal_mode) return;

    auto &current_ws = get_current_workspace(base);
    current_ws.scroll_offset = std::max(0, current_ws.scroll_offset - (SCROLL_STEP / 3));
    tile_horizontal(base);
}

void nwm::scroll_right(void *arg, Base &base) {
    (void)arg;
    if (!base.horizontal_mode) return;

    auto &current_ws = get_current_workspace(base);
    int screen_width = WIDTH(base.display, base.screen);
    int window_width = screen_width / 2;
    
    int total_width = current_ws.windows.size() * window_width;
    int max_scroll = std::max(0, total_width - screen_width);
    
    current_ws.scroll_offset = std::min(max_scroll, 
                                        current_ws.scroll_offset + (SCROLL_STEP / 3));
    tile_horizontal(base);
}

void nwm::manage_window(Window window, Base &base) {
    XWindowAttributes attr;

    if (XGetWindowAttributes(base.display, window, &attr) == 0) {
        std::cerr << "Warning: Window does not exist or is invalid\n";
        return;
    }

    if (attr.override_redirect) {
        return;
    }

    auto &current_ws = get_current_workspace(base);
    for (const auto &w : current_ws.windows) {
        if (w.window == window) {
            return;
        }
    }

    bool is_float = should_float(base.display, window);

    ManagedWindow w;
    w.window = window;
    w.is_floating = is_float;
    w.is_focused = false;
    w.workspace = base.current_workspace;
    
    if (is_float) {
        XSizeHints hints;
        long supplied;
        if (XGetWMNormalHints(base.display, window, &hints, &supplied)) {
            if (hints.flags & (PSize | USSize)) {
                w.width = hints.width;
                w.height = hints.height;
            } else {
                w.width = attr.width;
                w.height = attr.height;
            }
        } else {
            w.width = attr.width;
            w.height = attr.height;
        }
        
        int screen_width = WIDTH(base.display, base.screen);
        int screen_height = HEIGHT(base.display, base.screen);
        w.x = (screen_width - w.width) / 2;
        w.y = (screen_height - w.height) / 2;
        
        XMoveResizeWindow(base.display, window, w.x, w.y, w.width, w.height);
    } else {
        w.x = base.gaps;
        w.y = base.gaps + base.bar.height;
        w.width = WIDTH(base.display, base.screen) / 2;
        w.height = HEIGHT(base.display, base.screen) / 2;
    }

    current_ws.windows.push_back(w);

    XSetWindowAttributes attrs;
    attrs.event_mask = EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
                       StructureNotifyMask;
    XChangeWindowAttributes(base.display, window, CWEventMask, &attrs);
    XFlush(base.display);

    XSetWindowBorder(base.display, window, BORDER_COLOR);
    XSetWindowBorderWidth(base.display, window, is_float ? 1 : BORDER_WIDTH);

    XMapWindow(base.display, window);
    
    if (is_float) {
        XRaiseWindow(base.display, window);
    }
}

void nwm::unmanage_window(Window window, Base &base) {
    auto &current_ws = get_current_workspace(base);
    for (auto it = current_ws.windows.begin(); it != current_ws.windows.end(); ++it) {
        if (it->window == window) {
            if (current_ws.focused_window && current_ws.focused_window->window == window) {
                current_ws.focused_window = nullptr;
                base.focused_window = nullptr;
            }
            current_ws.windows.erase(it);
            
            if (base.horizontal_mode) {
                int screen_width = WIDTH(base.display, base.screen);
                int window_width = screen_width / 2;
                int total_width = current_ws.windows.size() * window_width;
                int max_scroll = std::max(0, total_width - screen_width);
                current_ws.scroll_offset = std::min(current_ws.scroll_offset, max_scroll);
            }
            break;
        }
    }
}

void nwm::focus_window(ManagedWindow *window, Base &base) {
    auto &current_ws = get_current_workspace(base);
    
    if (current_ws.focused_window) {
        XSetWindowBorder(base.display, current_ws.focused_window->window, BORDER_COLOR);
        current_ws.focused_window->is_focused = false;
    }

    current_ws.focused_window = nullptr;
    base.focused_window = nullptr;
    
    if (window) {
        current_ws.focused_window = window;
        base.focused_window = window;
        XSetWindowBorder(base.display, window->window, FOCUS_COLOR);
        XRaiseWindow(base.display, window->window);
        XSetInputFocus(base.display, window->window, RevertToPointerRoot, CurrentTime);
        window->is_focused = true;
    }
    XFlush(base.display);
}

void nwm::focus_next(void *arg, Base &base) {
    (void)arg;
    auto &current_ws = get_current_workspace(base);
    if (current_ws.windows.empty()) return;

    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) return;

    int current_idx = -1;
    for (size_t i = 0; i < tiled_windows.size(); ++i) {
        if (current_ws.focused_window && tiled_windows[i]->window == current_ws.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    int next_idx = (current_idx + 1) % tiled_windows.size();
    focus_window(tiled_windows[next_idx], base);
    
    if (base.horizontal_mode) {
        int screen_width = WIDTH(base.display, base.screen);
        int window_width = screen_width / 2;
        int target_scroll = next_idx * window_width;
        
        if (target_scroll < current_ws.scroll_offset) {
            current_ws.scroll_offset = target_scroll;
        } else if (target_scroll + window_width > current_ws.scroll_offset + screen_width) {
            current_ws.scroll_offset = target_scroll + window_width - screen_width;
        }
        
        tile_horizontal(base);
    }
}

void nwm::focus_prev(void *arg, Base &base) {
    (void)arg;
    auto &current_ws = get_current_workspace(base);
    if (current_ws.windows.empty()) return;

    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) return;

    int current_idx = -1;
    for (size_t i = 0; i < tiled_windows.size(); ++i) {
        if (current_ws.focused_window && tiled_windows[i]->window == current_ws.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    int prev_idx = (current_idx - 1 + tiled_windows.size()) % tiled_windows.size();
    focus_window(tiled_windows[prev_idx], base);
    
    if (base.horizontal_mode) {
        int screen_width = WIDTH(base.display, base.screen);
        int window_width = screen_width / 2;
        int target_scroll = prev_idx * window_width;
        
        if (target_scroll < current_ws.scroll_offset) {
            current_ws.scroll_offset = target_scroll;
        } else if (target_scroll + window_width > current_ws.scroll_offset + screen_width) {
            current_ws.scroll_offset = target_scroll + window_width - screen_width;
        }
        
        tile_horizontal(base);
    }
}

void nwm::swap_next(void *arg, Base &base) {
    (void)arg;
    auto &current_ws = get_current_workspace(base);
    if (current_ws.windows.size() < 2) return;

    int current_idx = -1;
    for (size_t i = 0; i < current_ws.windows.size(); ++i) {
        if (current_ws.focused_window && current_ws.windows[i].window == current_ws.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    if (current_idx == -1 || current_ws.windows[current_idx].is_floating) return;

    int next_idx = (current_idx + 1) % current_ws.windows.size();
    std::swap(current_ws.windows[current_idx], current_ws.windows[next_idx]);

    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    focus_window(&current_ws.windows[next_idx], base);
}

void nwm::swap_prev(void *arg, Base &base) {
    (void)arg;
    auto &current_ws = get_current_workspace(base);
    if (current_ws.windows.size() < 2) return;

    int current_idx = -1;
    for (size_t i = 0; i < current_ws.windows.size(); ++i) {
        if (current_ws.focused_window && current_ws.windows[i].window == current_ws.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    if (current_idx == -1 || current_ws.windows[current_idx].is_floating) return;

    int prev_idx = (current_idx - 1 + current_ws.windows.size()) % current_ws.windows.size();
    std::swap(current_ws.windows[current_idx], current_ws.windows[prev_idx]);

    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    focus_window(&current_ws.windows[prev_idx], base);
}

void nwm::resize_master(void *arg, Base &base) {
    auto &current_ws = get_current_workspace(base);
    if (current_ws.windows.size() < 2 || base.horizontal_mode) return;

    int delta = (int)(long)arg;
    int screen_width = WIDTH(base.display, base.screen);

    float delta_factor = (float)delta / screen_width;
    base.master_factor += delta_factor;

    if (base.master_factor < 0.1f) base.master_factor = 0.1f;
    if (base.master_factor > 0.9f) base.master_factor = 0.9f;

    tile_windows(base);
}

void nwm::move_window(ManagedWindow *window, int x, int y, Base &base) {
    if (window) {
        window->x = x;
        window->y = y;
        XMoveWindow(base.display, window->window, x, y);
    }
}

void nwm::resize_window(ManagedWindow *window, int width, int height, Base &base) {
    if (window) {
        window->width = width;
        window->height = height;
        XResizeWindow(base.display, window->window, width, height);
    }
}

void nwm::tile_horizontal(Base &base) {
    auto &current_ws = get_current_workspace(base);
    
    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) {
        for (auto &w : current_ws.windows) {
            if (w.is_floating) {
                XRaiseWindow(base.display, w.window);
            }
        }
        return;
    }

    int screen_width = WIDTH(base.display, base.screen);
    int screen_height = HEIGHT(base.display, base.screen);
    int bar_height = base.bar_visible ? base.bar.height : 0;
    int usable_height = screen_height - bar_height;

    if (tiled_windows.size() == 1) {
        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + bar_height;
        tiled_windows[0]->width = screen_width - 2 * base.gaps - 2 * BORDER_WIDTH;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * BORDER_WIDTH;
        
        XMoveResizeWindow(base.display, tiled_windows[0]->window,
                         tiled_windows[0]->x, tiled_windows[0]->y,
                         tiled_windows[0]->width, tiled_windows[0]->height);
        
        XRaiseWindow(base.display, tiled_windows[0]->window);
        XFlush(base.display);
        return;
    }

    int window_width = screen_width / 2;
    
    for (size_t i = 0; i < tiled_windows.size(); ++i) {
        int x_pos = i * window_width - current_ws.scroll_offset + base.gaps;
        int y_pos = base.gaps + bar_height;
        int win_width = window_width - 2 * base.gaps - 2 * BORDER_WIDTH;
        int win_height = usable_height - 2 * base.gaps - 2 * BORDER_WIDTH;
        
        tiled_windows[i]->x = x_pos;
        tiled_windows[i]->y = y_pos;
        tiled_windows[i]->width = win_width;
        tiled_windows[i]->height = win_height;

        XMoveResizeWindow(base.display, tiled_windows[i]->window,
                         tiled_windows[i]->x, tiled_windows[i]->y,
                         tiled_windows[i]->width, tiled_windows[i]->height);
    }
    
    for (auto &w : current_ws.windows) {
        if (w.is_floating) {
            XRaiseWindow(base.display, w.window);
        }
    }
    
    if (base.focused_window) {
        XRaiseWindow(base.display, base.focused_window->window);
    }
    
    XFlush(base.display);
}

void nwm::tile_windows(Base &base) {
    auto &current_ws = get_current_workspace(base);
    
    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) {
        for (auto &w : current_ws.windows) {
            if (w.is_floating) {
                XRaiseWindow(base.display, w.window);
            }
        }
        return;
    }

    int screen_width = WIDTH(base.display, base.screen);
    int screen_height = HEIGHT(base.display, base.screen);
    int bar_height = base.bar_visible ? base.bar.height : 0;
    int usable_height = screen_height - bar_height;

    if (tiled_windows.size() == 1) {
        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + bar_height;
        tiled_windows[0]->width = screen_width - 2 * base.gaps - 2 * BORDER_WIDTH;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * BORDER_WIDTH;
    } else {
        int master_width = (int)(screen_width * base.master_factor) - base.gaps - base.gaps / 2 - 2 * BORDER_WIDTH;
        int stack_x = (int)(screen_width * base.master_factor) + base.gaps / 2;
        int stack_width = screen_width - stack_x - base.gaps - 2 * BORDER_WIDTH;
        int stack_height = (usable_height - base.gaps * tiled_windows.size()) / (tiled_windows.size() - 1) - 2 * BORDER_WIDTH;

        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + bar_height;
        tiled_windows[0]->width = master_width;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * BORDER_WIDTH;

        for (size_t i = 1; i < tiled_windows.size(); ++i) {
            tiled_windows[i]->x = stack_x;
            tiled_windows[i]->y = base.gaps + bar_height + (i - 1) * (stack_height + base.gaps + 2 * BORDER_WIDTH);
            tiled_windows[i]->width = stack_width;
            tiled_windows[i]->height = stack_height;
        }
    }

    for (auto *w : tiled_windows) {
        XMoveResizeWindow(base.display, w->window, w->x, w->y, w->width, w->height);
    }
    
    for (auto *w : tiled_windows) {
        XRaiseWindow(base.display, w->window);
    }
    
    for (auto &w : current_ws.windows) {
        if (w.is_floating) {
            XRaiseWindow(base.display, w.window);
        }
    }
    
    if (base.focused_window) {
        XRaiseWindow(base.display, base.focused_window->window);
    }
    
    XFlush(base.display);
}

void nwm::close_window(void *arg, Base &base) {
    (void)arg;
    if (base.focused_window) {
        XEvent ev;
        ev.type = ClientMessage;
        ev.xclient.window = base.focused_window->window;
        ev.xclient.message_type = XInternAtom(base.display, "WM_PROTOCOLS", True);
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = XInternAtom(base.display, "WM_DELETE_WINDOW", False);
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(base.display, base.focused_window->window, False, NoEventMask, &ev);
    }
}

void nwm::quit_wm(void *arg, Base &base) {
    (void)arg;
    base.running = false;
}

void nwm::handle_map_request(XMapRequestEvent *e, Base &base) {
    manage_window(e->window, base);
    
    auto &current_ws = get_current_workspace(base);
    
    if (!current_ws.windows.empty()) {
        ManagedWindow *new_window = &current_ws.windows.back();
        
        if (!new_window->is_floating) {
            focus_window(new_window, base);
            
            if (base.horizontal_mode) {
                int screen_width = WIDTH(base.display, base.screen);
                int window_width = screen_width / 2;
                
                int tiled_idx = 0;
                for (size_t i = 0; i < current_ws.windows.size() - 1; ++i) {
                    if (!current_ws.windows[i].is_floating) {
                        tiled_idx++;
                    }
                }
                
                int target_scroll = tiled_idx * window_width;
                
                if (target_scroll + window_width > current_ws.scroll_offset + screen_width) {
                    current_ws.scroll_offset = target_scroll + window_width - screen_width;
                }
            }
        } else {
            XRaiseWindow(base.display, new_window->window);
        }
    }
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
}

void nwm::handle_unmap_notify(XUnmapEvent *e, Base &base) {
    unmanage_window(e->window, base);
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    
    auto &current_ws = get_current_workspace(base);
    if (!current_ws.windows.empty() && !current_ws.focused_window) {
        for (auto &w : current_ws.windows) {
            if (!w.is_floating) {
                focus_window(&w, base);
                break;
            }
        }
    }
}

void nwm::handle_destroy_notify(XDestroyWindowEvent *e, Base &base) {
    unmanage_window(e->window, base);
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }
    
    auto &current_ws = get_current_workspace(base);
    if (!current_ws.windows.empty() && !current_ws.focused_window) {
        for (auto &w : current_ws.windows) {
            if (!w.is_floating) {
                focus_window(&w, base);
                break;
            }
        }
    }
}

void nwm::handle_configure_request(XConfigureRequestEvent *e, Base &base) {
    XWindowChanges wc;
    wc.x = e->x;
    wc.y = e->y;
    wc.width = e->width;
    wc.height = e->height;
    wc.border_width = e->border_width;
    wc.sibling = e->above;
    wc.stack_mode = e->detail;

    auto &current_ws = get_current_workspace(base);
    bool is_floating = false;
    for (auto &w : current_ws.windows) {
        if (w.window == e->window && w.is_floating) {
            is_floating = true;
            break;
        }
    }
    
    if (is_floating) {
        XConfigureWindow(base.display, e->window, e->value_mask, &wc);
    } else {
        XConfigureWindow(base.display, e->window, e->value_mask & (CWSibling | CWStackMode), &wc);
    }
}

void nwm::handle_key_press(XKeyEvent *e, Base &base) {
    KeySym keysym = XLookupKeysym(e, 0);

    unsigned int cleaned_state = e->state & ~(LockMask | Mod2Mask);

    for (auto &k : keys) {
        unsigned int cleaned_mod = k.mod & ~(LockMask | Mod2Mask);
        if (keysym == k.keysym && cleaned_state == cleaned_mod) {
            if (k.func) {
                k.func((void*)k.arg, base);
            }
            break;
        }
    }
}

void nwm::reload_config(void *arg, nwm::Base &base) {
    (void)arg;
    std::cout << "Config reload triggered - please rebuild with 'make' for changes to take effect\n";
    std::cout << "After rebuilding, restart the WM or send SIGUSR1 signal\n";
    
    setup_keys(base);
    
    if (base.horizontal_mode) {
        tile_horizontal(base);
    } else {
        tile_windows(base);
    }

    XFlush(base.display);
}

void nwm::handle_button_press(XButtonEvent *e, Base &base) {
    if (e->window == base.bar.window) {
        return;
    }

    auto &current_ws = get_current_workspace(base);
    
    if (e->button == Button4 && (e->state & MODKEY)) {
        if (base.horizontal_mode) {
            scroll_left(nullptr, base);
        }
        return;
    }
    
    if (e->button == Button5 && (e->state & MODKEY)) {
        if (base.horizontal_mode) {
            scroll_right(nullptr, base);
        }
        return;
    }
    
    if (e->button == Button1 && (e->state & MODKEY)) {
        for (auto &w : current_ws.windows) {
            if (e->window == w.window || e->subwindow == w.window) {
                Window target_window = (e->window == w.window) ? e->window : e->subwindow;
                
                if (base.dragging || base.resizing) {
                    XUngrabPointer(base.display, CurrentTime);
                }
                
                base.dragging = true;
                base.resizing = false;
                base.drag_window = target_window;
                base.drag_start_x = e->x_root;
                base.drag_start_y = e->y_root;
                
                XWindowAttributes attr;
                if (XGetWindowAttributes(base.display, target_window, &attr)) {
                    base.drag_window_start_x = attr.x;
                    base.drag_window_start_y = attr.y;
                }
                
                focus_window(&w, base);
                XRaiseWindow(base.display, target_window);
                return;
            }
        }
    } else if (e->button == Button3 && (e->state & MODKEY)) {
        for (auto &w : current_ws.windows) {
            if (e->window == w.window || e->subwindow == w.window) {
                Window target_window = (e->window == w.window) ? e->window : e->subwindow;
                
                if (base.dragging || base.resizing) {
                    XUngrabPointer(base.display, CurrentTime);
                }
                
                base.resizing = true;
                base.dragging = false;
                base.drag_window = target_window;
                base.drag_start_x = e->x_root;
                base.drag_start_y = e->y_root;
                
                XWindowAttributes attr;
                if (XGetWindowAttributes(base.display, target_window, &attr)) {
                    base.resize_start_width = attr.width;
                    base.resize_start_height = attr.height;
                }
                
                focus_window(&w, base);
                XRaiseWindow(base.display, target_window);
                return;
            }
        }
    } else if (e->button == Button1) {
        for (auto &w : current_ws.windows) {
            if (e->window == w.window || e->subwindow == w.window) {
                focus_window(&w, base);
                break;
            }
        }
    }
}

void nwm::handle_button_release(XButtonEvent *e, Base &base) {
    if (!base.dragging && !base.resizing) return;
    
    if (e->button == Button1 || e->button == Button3) {
        XUngrabPointer(base.display, CurrentTime);
        
        auto &current_ws = get_current_workspace(base);
        
        if (base.dragging && base.horizontal_mode && current_ws.windows.size() > 1) {
            int dragged_idx = -1;
            bool is_floating = false;
            
            for (size_t i = 0; i < current_ws.windows.size(); ++i) {
                if (current_ws.windows[i].window == base.drag_window) {
                    dragged_idx = i;
                    is_floating = current_ws.windows[i].is_floating;
                    break;
                }
            }
            
            if (dragged_idx != -1 && !is_floating) {
                int screen_width = WIDTH(base.display, base.screen);
                int window_width = screen_width / 2;
                
                int window_center_x = e->x_root;
                
                int target_idx = (window_center_x + current_ws.scroll_offset) / window_width;
                
                if (target_idx < 0) target_idx = 0;
                if (target_idx >= (int)current_ws.windows.size()) target_idx = current_ws.windows.size() - 1;
                
                if (target_idx != dragged_idx) {
                    ManagedWindow dragged_window = current_ws.windows[dragged_idx];
                    current_ws.windows.erase(current_ws.windows.begin() + dragged_idx);
                    current_ws.windows.insert(current_ws.windows.begin() + target_idx, dragged_window);
                }
            }
            
            tile_horizontal(base);
        } else if (base.resizing) {
            bool is_floating = false;
            for (auto &w : current_ws.windows) {
                if (w.window == base.drag_window) {
                    is_floating = w.is_floating;
                    break;
                }
            }
            
            if (!is_floating && !base.horizontal_mode && current_ws.windows.size() >= 2) {
                for (size_t i = 0; i < current_ws.windows.size(); ++i) {
                    if (current_ws.windows[i].window == base.drag_window && i == 0) {
                        XWindowAttributes attr;
                        if (XGetWindowAttributes(base.display, base.drag_window, &attr)) {
                            int screen_width = WIDTH(base.display, base.screen);
                            base.master_factor = (float)attr.width / screen_width;
                            if (base.master_factor < 0.1f) base.master_factor = 0.1f;
                            if (base.master_factor > 0.9f) base.master_factor = 0.9f;
                        }
                        break;
                    }
                }
            }
            tile_windows(base);
        } else {
            if (base.horizontal_mode) {
                tile_horizontal(base);
            } else {
                tile_windows(base);
            }
        }
        
        base.dragging = false;
        base.resizing = false;
        base.drag_window = None;
        
        if (base.focused_window) {
            XRaiseWindow(base.display, base.focused_window->window);
        }
        
        XFlush(base.display);
    }
}

void nwm::handle_motion_notify(XMotionEvent *e, Base &base) {
    if (!base.dragging && !base.resizing) return;
    if (base.drag_window == None) return;
    
    auto &current_ws = get_current_workspace(base);
    
    if (base.dragging) {
        for (auto &w : current_ws.windows) {
            if (w.window == base.drag_window) {
                int delta_x = e->x_root - base.drag_start_x;
                int delta_y = e->y_root - base.drag_start_y;
                
                int new_x = base.drag_window_start_x + delta_x;
                int new_y = base.drag_window_start_y + delta_y;
                
                XMoveWindow(base.display, w.window, new_x, new_y);
                XRaiseWindow(base.display, w.window);
                break;
            }
        }
    } else if (base.resizing) {
        for (auto &w : current_ws.windows) {
            if (w.window == base.drag_window) {
                int delta_x = e->x_root - base.drag_start_x;
                int delta_y = e->y_root - base.drag_start_y;
                
                int new_width = base.resize_start_width + delta_x;
                int new_height = base.resize_start_height + delta_y;
                
                if (new_width < 100) new_width = 100;
                if (new_height < 100) new_height = 100;
                
                XResizeWindow(base.display, w.window, new_width, new_height);
                XRaiseWindow(base.display, w.window);
                break;
            }
        }
    }
}

void nwm::handle_enter_notify(XCrossingEvent *e, Base &base) {
    if (e->window == base.bar.window) {
        return;
    }
    
    if (base.dragging || base.resizing) {
        return;
    }
    
    auto &current_ws = get_current_workspace(base);
    for (auto &w : current_ws.windows) {
        if (e->window == w.window) {
            focus_window(&w, base);
            break;
        }
    }
}

void nwm::handle_expose(XExposeEvent *e, Base &base) {
    if (e->window == base.bar.window) {
        bar_draw(base);
    }
}

void nwm::init(Base &base) {
    struct sigaction sa;
    sa.sa_handler = [](int) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
    };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    base.display = XOpenDisplay(NULL);
    if (!base.display) {
        std::cerr << "Error: Cannot open display\n";
        std::exit(1);
    }

    XSetErrorHandler(x_error_handler);

    base.gaps_enabled = true;
    base.gaps = GAP_SIZE;
    base.screen = DefaultScreen(base.display);
    base.root = RootWindow(base.display, base.screen);
    base.focused_window = nullptr;
    base.running = false;
    base.master_factor = 0.5f;
    base.horizontal_mode = false;
    base.widget = WIDGET;
    base.bar_visible = true;
    
    base.cursor = XCreateFontCursor(base.display, XC_left_ptr);
    XDefineCursor(base.display, base.root, base.cursor);

    base.xft_draw = XftDrawCreate(base.display, base.root,
                                 DefaultVisual(base.display, base.screen),
                                 DefaultColormap(base.display, base.screen));
    if (!base.xft_draw) {
        std::cerr << "Error: Failed to create XftDraw\n";
        std::exit(1);
    }

    base.xft_font = XftFontOpenName(base.display, base.screen, FONT);
    if (!base.xft_font) {
        std::cerr << "Warning: Failed to load font '" << FONT << "', trying fallback\n";
        base.xft_font = XftFontOpenName(base.display, base.screen, "monospace:size=10");
    }
    if (!base.xft_font) {
        base.xft_font = XftFontOpenName(base.display, base.screen, "fixed");
    }
    if (!base.xft_font) {
        std::cerr << "Error: Failed to load any Xft font\n";
        std::exit(1);
    }

    workspace_init(base);
    bar_init(base);

    XSelectInput(base.display, base.root,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 ButtonPressMask | EnterWindowMask | KeyPressMask | PropertyChangeMask);

    Window root_return, parent_return;
    Window *children;
    unsigned int nchildren;

    if (XQueryTree(base.display, base.root, &root_return, &parent_return, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; ++i) {
            XWindowAttributes attr;
            if (XGetWindowAttributes(base.display, children[i], &attr)) {
                if (!attr.override_redirect && attr.map_state == IsViewable) {
                    nwm::manage_window(children[i], base);
                }
            }
        }
        if (children) XFree(children);
    }

    nwm::tile_windows(base);
    nwm::setup_keys(base);
    bar_draw(base);
}

void nwm::cleanup(Base &base) {
    if (base.dragging) {
        XUngrabPointer(base.display, CurrentTime);
        base.dragging = false;
        base.drag_window = None;
    }
    
    for (auto &ws : base.workspaces) {
        for (auto &w : ws.windows) {
            XUnmapWindow(base.display, w.window);
        }
    }

    bar_cleanup(base);

    if (base.xft_font) {
        XftFontClose(base.display, base.xft_font);
        base.xft_font = nullptr;
    }

    if (base.xft_draw) {
        XftDrawDestroy(base.xft_draw);
        base.xft_draw = nullptr;
    }

    if (base.cursor) {
        XFreeCursor(base.display, base.cursor);
        base.cursor = 0;
    }

    if (base.display) {
        XCloseDisplay(base.display);
        base.display = nullptr;
    }
}

void nwm::run(Base &base) {
    base.running = true;

    XSelectInput(base.display, base.root,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                 EnterWindowMask | KeyPressMask | PropertyChangeMask);

    XSetErrorHandler(x_error_handler);

    time_t last_bar_update = time(nullptr);

    while (base.running) {
        while (XPending(base.display)) {
            XEvent e;
            XNextEvent(base.display, &e);

            switch (e.type) {
                case MapRequest:
                    handle_map_request(&e.xmaprequest, base);
                    break;
                case UnmapNotify:
                    handle_unmap_notify(&e.xunmap, base);
                    break;
                case DestroyNotify:
                    handle_destroy_notify(&e.xdestroywindow, base);
                    break;
                case ConfigureRequest:
                    handle_configure_request(&e.xconfigurerequest, base);
                    break;
                case KeyPress:
                    handle_key_press(&e.xkey, base);
                    break;
                case ButtonPress:
                    handle_button_press(&e.xbutton, base);
                    break;
                case ButtonRelease:
                    handle_button_release(&e.xbutton, base);
                    break;
                case MotionNotify:
                    handle_motion_notify(&e.xmotion, base);
                    break;
                case EnterNotify:
                    handle_enter_notify(&e.xcrossing, base);
                    break;
                case Expose:
                    handle_expose(&e.xexpose, base);
                    break;
                default:
                    break;
            }
        }

        time_t now = time(nullptr);
        if (now - last_bar_update >= 60) {
            bar_update_time(base);
            last_bar_update = now;
        }

        usleep(10000);
    }
}

int main(void) {
    nwm::Base wm;
    nwm::init(wm);
    nwm::run(wm);
    nwm::cleanup(wm);
    return 0;
}
