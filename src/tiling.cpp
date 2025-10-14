#include "tiling.hpp"
#include "bar.hpp"
#include <algorithm>
#include <X11/Xlib.h>

static void atomic_restack(Display *display, nwm::Base &base, std::vector<Window> &stack_order) {
    (void)base;
    if (stack_order.empty()) return;
    XRestackWindows(display, stack_order.data(), stack_order.size());
}

static void raise_override_windows(Display *display, nwm::Base &base) {
    Window root = DefaultRootWindow(display);
    Window root_return, parent_return;
    Window *children;
    unsigned int nchildren;
    
    if (!XQueryTree(display, root, &root_return, &parent_return, &children, &nchildren)) {
        return;
    }
    
    int screen_width = WIDTH(display, base.screen);
    int screen_height = HEIGHT(display, base.screen);
    
    for (unsigned int i = 0; i < nchildren; ++i) {
        XWindowAttributes attr;
        if (XGetWindowAttributes(display, children[i], &attr)) {
            if (attr.override_redirect && attr.map_state == IsViewable) {
                if (children[i] != base.bar.window && children[i] != base.systray.window) {
                    bool is_large = (attr.width > screen_width / 4 && attr.height > screen_height / 4);
                    
                    if (is_large) {
                        XRaiseWindow(display, children[i]);
                    }
                }
            }
        }
    }
    
    if (children) {
        XFree(children);
    }
}

void nwm::tile_horizontal(Base &base) {
    auto &current_ws = get_current_workspace(base);
    
    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating && !w.is_fullscreen) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) {
        std::vector<Window> stack_order;
        for (auto &w : current_ws.windows) {
            if (w.is_floating || w.is_fullscreen) {
                stack_order.push_back(w.window);
            }
        }
        if (base.focused_window && (base.focused_window->is_floating || base.focused_window->is_fullscreen)) {
            auto it = std::find(stack_order.begin(), stack_order.end(), base.focused_window->window);
            if (it != stack_order.end()) {
                stack_order.erase(it);
                stack_order.push_back(base.focused_window->window);
            }
            XSetInputFocus(base.display, base.focused_window->window, RevertToPointerRoot, CurrentTime);
        }
        atomic_restack(base.display, base, stack_order);
        raise_override_windows(base.display, base);
        XFlush(base.display);
        return;
    }

    int screen_width = WIDTH(base.display, base.screen);
    int screen_height = HEIGHT(base.display, base.screen);
    int bar_height = base.bar_visible ? base.bar.height : 0;
    int usable_height = screen_height - bar_height;
    int y_start = (base.bar_position == 0) ? bar_height : 0;

    std::vector<Window> stack_order;

    if (tiled_windows.size() == 1) {
        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + y_start;
        tiled_windows[0]->width = screen_width - 2 * base.gaps - 2 * base.border_width;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * base.border_width;
        
        XMoveResizeWindow(base.display, tiled_windows[0]->window,
                         tiled_windows[0]->x, tiled_windows[0]->y,
                         tiled_windows[0]->width, tiled_windows[0]->height);
        
        stack_order.push_back(tiled_windows[0]->window);
    } else {
        int window_width = screen_width / 2;
        
        for (size_t i = 0; i < tiled_windows.size(); ++i) {
            int x_pos = i * window_width - current_ws.scroll_offset + base.gaps;
            int y_pos = base.gaps + y_start;
            int win_width = window_width - 2 * base.gaps - 2 * base.border_width;
            int win_height = usable_height - 2 * base.gaps - 2 * base.border_width;
            
            tiled_windows[i]->x = x_pos;
            tiled_windows[i]->y = y_pos;
            tiled_windows[i]->width = win_width;
            tiled_windows[i]->height = win_height;

            XMoveResizeWindow(base.display, tiled_windows[i]->window,
                             tiled_windows[i]->x, tiled_windows[i]->y,
                             tiled_windows[i]->width, tiled_windows[i]->height);
            
            stack_order.push_back(tiled_windows[i]->window);
        }
    }
    
    for (auto &w : current_ws.windows) {
        if (w.is_floating || w.is_fullscreen) {
            stack_order.push_back(w.window);
        }
    }
    
    if (base.focused_window && (base.focused_window->is_floating || base.focused_window->is_fullscreen)) {
        auto it = std::find(stack_order.begin(), stack_order.end(), base.focused_window->window);
        if (it != stack_order.end()) {
            stack_order.erase(it);
            stack_order.push_back(base.focused_window->window);
        }
    }
    
    atomic_restack(base.display, base, stack_order);
    
    if (base.focused_window && (base.focused_window->is_floating || base.focused_window->is_fullscreen)) {
        XSetInputFocus(base.display, base.focused_window->window, RevertToPointerRoot, CurrentTime);
    }
    
    raise_override_windows(base.display, base);
    
    XFlush(base.display);
}

void nwm::tile_windows(Base &base) {
    auto &current_ws = get_current_workspace(base);
    
    std::vector<ManagedWindow*> tiled_windows;
    for (auto &w : current_ws.windows) {
        if (!w.is_floating && !w.is_fullscreen) {
            tiled_windows.push_back(&w);
        }
    }
    
    if (tiled_windows.empty()) {
        std::vector<Window> stack_order;
        for (auto &w : current_ws.windows) {
            if (w.is_floating || w.is_fullscreen) {
                stack_order.push_back(w.window);
            }
        }
        if (base.focused_window) {
            auto it = std::find(stack_order.begin(), stack_order.end(), base.focused_window->window);
            if (it != stack_order.end()) {
                stack_order.erase(it);
                stack_order.push_back(base.focused_window->window);
            }
        }
        atomic_restack(base.display, base, stack_order);
        raise_override_windows(base.display, base);
        XFlush(base.display);
        return;
    }

    int screen_width = WIDTH(base.display, base.screen);
    int screen_height = HEIGHT(base.display, base.screen);
    int bar_height = base.bar_visible ? base.bar.height : 0;
    int usable_height = screen_height - bar_height;
    int y_start = (base.bar_position == 0) ? bar_height : 0;

    std::vector<Window> stack_order;

    if (tiled_windows.size() == 1) {
        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + y_start;
        tiled_windows[0]->width = screen_width - 2 * base.gaps - 2 * base.border_width;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * base.border_width;
        
        XMoveResizeWindow(base.display, tiled_windows[0]->window,
                         tiled_windows[0]->x, tiled_windows[0]->y,
                         tiled_windows[0]->width, tiled_windows[0]->height);
        
        stack_order.push_back(tiled_windows[0]->window);
    } else {
        int master_width = (int)(screen_width * base.master_factor) - base.gaps - base.gaps / 2 - 2 * base.border_width;
        int stack_x = (int)(screen_width * base.master_factor) + base.gaps / 2;
        int stack_width = screen_width - stack_x - base.gaps - 2 * base.border_width;
        int stack_height = (usable_height - base.gaps * tiled_windows.size()) / (tiled_windows.size() - 1) - 2 * base.border_width;

        tiled_windows[0]->x = base.gaps;
        tiled_windows[0]->y = base.gaps + y_start;
        tiled_windows[0]->width = master_width;
        tiled_windows[0]->height = usable_height - 2 * base.gaps - 2 * base.border_width;

        for (size_t i = 1; i < tiled_windows.size(); ++i) {
            tiled_windows[i]->x = stack_x;
            tiled_windows[i]->y = base.gaps + y_start + (i - 1) * (stack_height + base.gaps + 2 * base.border_width);
            tiled_windows[i]->width = stack_width;
            tiled_windows[i]->height = stack_height;
        }

        for (auto *w : tiled_windows) {
            XMoveResizeWindow(base.display, w->window, w->x, w->y, w->width, w->height);
            stack_order.push_back(w->window);
        }
    }
    
    for (auto &w : current_ws.windows) {
        if (w.is_floating || w.is_fullscreen) {
            stack_order.push_back(w.window);
        }
    }
    
    atomic_restack(base.display, base, stack_order);
    
    raise_override_windows(base.display, base);
    
    XFlush(base.display);
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

void nwm::scroll_left(void *arg, Base &base) {
    (void)arg;
    if (!base.horizontal_mode) return;

    auto &current_ws = get_current_workspace(base);
    current_ws.scroll_offset = std::max(0, current_ws.scroll_offset - (base.scroll_step / 3));
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
                                        current_ws.scroll_offset + (base.scroll_step / 3));
    tile_horizontal(base);
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
    
    bar_draw(base);
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
