#include "nwm.hpp"
#include "api.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

void nwm::manage_window(Window window, Base &base) {
  ManagedWindow w;
  w.window = window;
  w.x = 0;
  w.y = 0;
  w.width = WIDTH(base.display, base.screen) / 2;
  w.height = HEIGHT(base.display, base.screen) / 2;
  w.is_floating = false;
  w.is_focused = false;

  base.windows.push_back(w);

  // Set window attributes
  XSetWindowAttributes attrs;
  attrs.event_mask = EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
                     StructureNotifyMask | SubstructureRedirectMask;
  XChangeWindowAttributes(base.display, window, CWEventMask, &attrs);

  // Add border
  XSetWindowBorder(base.display, window, BORDER_WIDTH);
  XSetWindowBorderWidth(base.display, window, BORDER_WIDTH);

  // Map the window
  XMapWindow(base.display, window);
}

void nwm::unmanage_window(Window window, Base &base) {
  for (auto it = base.windows.begin(); it != base.windows.end(); ++it) {
    if (it->window == window) {
      base.windows.erase(it);
      break;
    }
  }
}

void nwm::focus_window(ManagedWindow *window, Base &base) {
  if (base.focused_window) {
    XSetWindowBorder(base.display, base.focused_window->window, BORDER_WIDTH);
    base.focused_window->is_focused = false;
  }

  base.focused_window = window;
  if (window) {
    XSetWindowBorder(base.display, window->window, BORDER_WIDTH * 2);
    XSetInputFocus(base.display, window->window, RevertToPointerRoot,
                   CurrentTime);
    window->is_focused = true;
  }
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
  if (num_windows == 0)
    return;

  int screen_width = WIDTH(base.display, base.screen);
  int screen_height = HEIGHT(base.display, base.screen);
  int window_width = screen_width / num_windows;

  for (size_t i = 0; i < base.windows.size(); ++i) {
    base.windows[i].x = i * window_width;
    base.windows[i].y = 0;
    base.windows[i].width = window_width;
    base.windows[i].height = screen_height;

    XMoveResizeWindow(base.display, base.windows[i].window, base.windows[i].x,
                      base.windows[i].y, base.windows[i].width,
                      base.windows[i].height);
  }
}

void nwm::handle_map_request(XMapRequestEvent *e, Base &base,
                        application::app &apps) {
  manage_window(e->window, base);
  tile_windows(base);
}

void nwm::handle_unmap_notify(XUnmapEvent *e, Base &base) {
  unmanage_window(e->window, base);
  tile_windows(base);
}

void nwm::handle_destroy_notify(XDestroyWindowEvent *e, Base &base) {
  unmanage_window(e->window, base);
  tile_windows(base);
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

void nwm::handle_key_press(XKeyEvent *e, Base &base, application::app &apps) {
  // TODO: Implement key press handling
  // This should handle keyboard shortcuts and window management commands
}

void nwm::handle_button_press(XButtonEvent *e, Base &base, application::app &apps) {
  // TODO: Implement button press handling
  // This should handle mouse button events for window management
}

void nwm::update(De &env, Base &base, application::app &apps) {
  // Set up event mask for root window
  XSelectInput(base.display, base.root,
               SubstructureRedirectMask | SubstructureNotifyMask |
                   ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                   EnterWindowMask | LeaveWindowMask | KeyPressMask |
                   PropertyChangeMask);

  // Initialize focused window
  base.focused_window = nullptr;

  // Load Lua configuration
  load_lua(apps);

  // Main event loop
  while (true) {
    XNextEvent(base.display, env.event);

    switch (env.event->type) {
    case MapRequest:
      handle_map_request(&env.event->xmaprequest, base, apps);
      break;
    case UnmapNotify:
      handle_unmap_notify(&env.event->xunmap, base);
      break;
    case DestroyNotify:
      handle_destroy_notify(&env.event->xdestroywindow, base);
      break;
    case ConfigureRequest:
      handle_configure_request(&env.event->xconfigurerequest, base);
      break;
    case KeyPress:
      handle_key_press(&env.event->xkey, base, apps);
      break;
    case ButtonPress:
      handle_button_press(&env.event->xbutton, base, apps);
      break;
    }
  }
}

void nwm::file_check(Base &test) {
  std::string cmd = "touch ";
  std::string config = "~/.config/nwm/nwm.lua ";
  cmd = cmd + config;
  system(cmd.c_str());
  std::cout << "File created successfully" << std::endl;
}

void nwm::init(Base &test) {
  test.display = XOpenDisplay(NULL);
  if (test.display == NULL) {
    std::cerr << "Display connection could not be initialized\n";
    std::exit(1);
  }
  test.screen = DefaultScreen(test.display);
  test.root = RootWindow(test.display, test.screen);
}

void nwm::clean(Base &test, De &env) {
  XUnmapWindow(test.display, env.window);
  XDestroyWindow(test.display, env.window);
  XCloseDisplay(test.display);
}

int main(void) {
  setenv("DISPLAY", ":0", 1);
  application::app a;
  nwm::De x;
  nwm::Base y;
  nwm::init(y);
  nwm::update(x, y, a);
  nwm::clean(y, x);
  return 0;
}
