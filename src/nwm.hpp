#ifndef NWM_HPP
#define NWM_HPP

/* Include stuff */
#include "api.hpp"
#include <X11/Xlib.h>
#include <fstream>

#define BORDER 0
#define SCREEN_NUMBER 0
#define POSITION_X 0
#define POSITION_Y 0

#define WIDTH(display, screen_number) XDisplayWidth((display), (screen_number))
#define HEIGHT(display, screen_number)                                         \
  XDisplayHeight((display), (screen_number))

namespace nwm {
struct Base {
  int screen;
  Window root;
  Display *display;
  std::fstream config;
  std::string error;
};
struct De {
  Window window;
  XEvent *event;
};
} // namespace nwm

// Start stuff
void update(nwm::De &, nwm::Base &, application::app &);
void init(nwm::Base &);
void clean(nwm::Base &, nwm::De &);
void file_check(nwm::Base &);

// Event handlers
void handle_key_press(XKeyEvent *, nwm::Base &, application::app &);
void handle_button_press(XButtonEvent *, nwm::Base &, application::app &);
void handle_configure_request(XConfigureRequestEvent *, nwm::Base &);
void handle_map_request(XMapRequestEvent *, nwm::Base &, application::app &);

#endif // NWM_HPP
