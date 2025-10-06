#include "nwm.hpp"
#include "config.hpp"
#include "util.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

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

int x_error_handler(Display *dpy, XErrorEvent *error) {
    char error_text[1024];
    XGetErrorText(dpy, error->error_code, error_text, sizeof(error_text));
    std::cerr << "X Error: " << error_text
              << " Request code: " << (int)error->request_code
              << " Resource ID: " << error->resourceid << std::endl;
    return 0;
}

void nwm::manage_window(Window window, Base &base) {
    XWindowAttributes attr;

    if (XGetWindowAttributes(base.display, window, &attr) == 0) {
        std::cerr << "Warning: Window does not exist or is invalid\n";
        return;
    }

    // ignore for menu and stuff like that
    if (attr.override_redirect) {
        return;
    }

    for (const auto &w : base.windows) {
        if (w.window == window) {
            return;
        }
    }

    ManagedWindow w;
    w.window = window;
    w.x = GAP_SIZE;
    w.y = GAP_SIZE;
    w.width = WIDTH(base.display, base.screen) / 2;
    w.height = HEIGHT(base.display, base.screen) / 2;
    w.is_floating = false;
    w.is_focused = false;

    base.windows.push_back(w);

    // set window attributes
    XSetWindowAttributes attrs;
    attrs.event_mask = EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
                       StructureNotifyMask;
    XChangeWindowAttributes(base.display, window, CWEventMask, &attrs);
    XFlush(base.display);

    XSetWindowBorder(base.display, window, BORDER_COLOR);
    XSetWindowBorderWidth(base.display, window, BORDER_WIDTH);

    XMapWindow(base.display, window);
}

void nwm::unmanage_window(Window window, Base &base) {
    for (auto it = base.windows.begin(); it != base.windows.end(); ++it) {
        if (it->window == window) {
            if (base.focused_window && base.focused_window->window == window) {
                base.focused_window = nullptr;
            }
            base.windows.erase(it);
            break;
        }
    }
}

void nwm::focus_window(ManagedWindow *window, Base &base) {
    if (base.focused_window) {
        XSetWindowBorder(base.display, base.focused_window->window, BORDER_COLOR);
        base.focused_window->is_focused = false;
    }

    base.focused_window = nullptr;
    if (window) {
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
    if (base.windows.empty()) return;

    int current_idx = -1;
    for (size_t i = 0; i < base.windows.size(); ++i) {
        if (base.focused_window && base.windows[i].window == base.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    int next_idx = (current_idx + 1) % base.windows.size();
    focus_window(&base.windows[next_idx], base);
}

void nwm::focus_prev(void *arg, Base &base) {
    (void)arg;
    if (base.windows.empty()) return;

    int current_idx = -1;
    for (size_t i = 0; i < base.windows.size(); ++i) {
        if (base.focused_window && base.windows[i].window == base.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    int prev_idx = (current_idx - 1 + base.windows.size()) % base.windows.size();
    focus_window(&base.windows[prev_idx], base);
}

void nwm::swap_next(void *arg, Base &base) {
    (void)arg;
    if (base.windows.size() < 2) return;

    int current_idx = -1;
    for (size_t i = 0; i < base.windows.size(); ++i) {
        if (base.focused_window && base.windows[i].window == base.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    if (current_idx == -1) return;

    int next_idx = (current_idx + 1) % base.windows.size();
    std::swap(base.windows[current_idx], base.windows[next_idx]);

    tile_windows(base);
    // Keep focus on the same window (which is now at next_idx)
    focus_window(&base.windows[next_idx], base);
}

void nwm::swap_prev(void *arg, Base &base) {
    (void)arg;
    if (base.windows.size() < 2) return;

    int current_idx = -1;
    for (size_t i = 0; i < base.windows.size(); ++i) {
        if (base.focused_window && base.windows[i].window == base.focused_window->window) {
            current_idx = i;
            break;
        }
    }

    if (current_idx == -1) return;

    int prev_idx = (current_idx - 1 + base.windows.size()) % base.windows.size();
    std::swap(base.windows[current_idx], base.windows[prev_idx]);

    tile_windows(base);
    // Keep focus on the same window (which is now at prev_idx)
    focus_window(&base.windows[prev_idx], base);
}

void nwm::resize_master(void *arg, Base &base) {
    if (base.windows.size() < 2) return;

    int delta = (int)(long)arg;
    int screen_width = WIDTH(base.display, base.screen);

    // Calculate new master factor based on pixel delta
    float delta_factor = (float)delta / screen_width;
    base.master_factor += delta_factor;

    // Clamp between 0.1 and 0.9 for reasonable bounds
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

void nwm::tile_windows(Base &base) {
    int num_windows = base.windows.size();
    if (num_windows == 0) return;

    int screen_width = WIDTH(base.display, base.screen);
    int screen_height = HEIGHT(base.display, base.screen);

    if (num_windows == 1) {
        // single window takes full screen with gaps
        base.windows[0].x = GAP_SIZE;
        base.windows[0].y = GAP_SIZE;
        base.windows[0].width = screen_width - 2 * GAP_SIZE - 2 * BORDER_WIDTH;
        base.windows[0].height = screen_height - 2 * GAP_SIZE - 2 * BORDER_WIDTH;
    } else {
        // master-stack layout with adjustable master size
        int master_width = (int)(screen_width * base.master_factor) - GAP_SIZE - GAP_SIZE / 2 - 2 * BORDER_WIDTH;
        int stack_x = (int)(screen_width * base.master_factor) + GAP_SIZE / 2;
        int stack_width = screen_width - stack_x - GAP_SIZE - 2 * BORDER_WIDTH;
        int stack_height = (screen_height - GAP_SIZE * num_windows) / (num_windows - 1) - 2 * BORDER_WIDTH;

        // master window (left)
        base.windows[0].x = GAP_SIZE;
        base.windows[0].y = GAP_SIZE;
        base.windows[0].width = master_width;
        base.windows[0].height = screen_height - 2 * GAP_SIZE - 2 * BORDER_WIDTH;

        // stack windows (right)
        for (size_t i = 1; i < base.windows.size(); ++i) {
            base.windows[i].x = stack_x;
            base.windows[i].y = GAP_SIZE + (i - 1) * (stack_height + GAP_SIZE + 2 * BORDER_WIDTH);
            base.windows[i].width = stack_width;
            base.windows[i].height = stack_height;
        }
    }

    for (auto &w : base.windows) {
        XMoveResizeWindow(base.display, w.window, w.x, w.y, w.width, w.height);
    }
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
    tile_windows(base);
    focus_window(&base.windows.back(), base);
}

void nwm::handle_unmap_notify(XUnmapEvent *e, Base &base) {
    unmanage_window(e->window, base);
    tile_windows(base);
    if (!base.windows.empty() && !base.focused_window) {
        focus_window(&base.windows[0], base);
    }
}

void nwm::handle_destroy_notify(XDestroyWindowEvent *e, Base &base) {
    unmanage_window(e->window, base);
    tile_windows(base);
    if (!base.windows.empty() && !base.focused_window) {
        focus_window(&base.windows[0], base);
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

    XConfigureWindow(base.display, e->window, e->value_mask, &wc);
}

void nwm::handle_key_press(XKeyEvent *e, Base &base) {
    KeySym keysym = XLookupKeysym(e, 0);

    // Clean the state to ignore NumLock and CapsLock
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
    std::cout << "Hot reloading configuration...\n";

    // Save current state
    std::vector<Window> window_ids;
    for (const auto &w : base.windows) {
        window_ids.push_back(w.window);
    }
    Window focused = base.focused_window ? base.focused_window->window : None;
    float master_factor = base.master_factor;

    // Reload keybindings
    setup_keys(base);

    // Restore master factor in case config changed it
    base.master_factor = master_factor;

    // Retile with new configuration
    tile_windows(base);

    // Restore focus
    if (focused != None) {
        for (auto &w : base.windows) {
            if (w.window == focused) {
                focus_window(&w, base);
                break;
            }
        }
    }

    XFlush(base.display);
    std::cout << "Configuration reloaded successfully!\n";
}

void nwm::handle_button_press(XButtonEvent *e, Base &base) {
    if (e->button == Button1) {
        for (auto &w : base.windows) {
            if (e->window == w.window) {
                focus_window(&w, base);
                break;
            }
        }
    }
}

void nwm::handle_enter_notify(XCrossingEvent *e, Base &base) {
    for (auto &w : base.windows) {
        if (e->window == w.window) {
            focus_window(&w, base);
            break;
        }
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

    base.screen = DefaultScreen(base.display);
    base.root = RootWindow(base.display, base.screen);
    base.focused_window = nullptr;
    base.running = false;
    base.master_factor = 0.5f; // Default 50/50 split
    base.cursor = XCreateFontCursor(base.display, XC_left_ptr);
    XDefineCursor(base.display, base.root, base.cursor);

    base.xft_draw = XftDrawCreate(base.display, base.root,
                                 DefaultVisual(base.display, base.screen),
                                 DefaultColormap(base.display, base.screen));
    if (!base.xft_draw) {
        std::cerr << "Error: Failed to create XftDraw\n";
        std::exit(1);
    }

    base.xft_font = XftFontOpenName(base.display, base.screen, "monospace:size=12");
    if (!base.xft_font) {
        base.xft_font = XftFontOpenName(base.display, base.screen, "fixed");
    }
    if (!base.xft_font) {
        std::cerr << "Error: Failed to load Xft font\n";
        std::exit(1);
    }

    XSelectInput(base.display, base.root,
                 SubstructureRedirectMask | SubstructureNotifyMask |
                 ButtonPressMask | EnterWindowMask | KeyPressMask);

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
}

void nwm::cleanup(Base &base) {
    for (auto &w : base.windows) {
        XUnmapWindow(base.display, w.window);
    }

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
                 ButtonPressMask | EnterWindowMask | KeyPressMask);

    XSetErrorHandler(x_error_handler);

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
                case EnterNotify:
                    handle_enter_notify(&e.xcrossing, base);
                    break;
                default:
                    break;
            }
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
