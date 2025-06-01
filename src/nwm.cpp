#include "nwm.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <iostream>

void update(Window window, XEvent event, nwm::UI_elt &test){
    window = XCreateSimpleWindow(test.display, test.root, POSITION_X, POSITION_Y,  WIDTH, HEIGHT, BORDER ,BlackPixel(test.display, test.screen), BlackPixel(test.display, test.screen));
    XMapWindow(test.display, window);
    while (XNextEvent(test.display, &event) == 0) {

    }
}

void init(nwm::UI_elt &test){
    test.display = XOpenDisplay(NULL);
    if (test.display == NULL) {
        std::cerr << "Diplay connection could not be initalized";
    }
    test.screen = DefaultScreen(test.display);
    test.root = RootWindow(test.display, test.screen);
}

void clean(nwm::UI_elt &test, Window window){
    XUnmapWindow(test.display, window);
    XDestroyWindow(test.display, window);
    XCloseDisplay(test.display);
}
