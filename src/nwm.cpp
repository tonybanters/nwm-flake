#include "nwm.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <sys/stat.h>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>

Display* nwm::Base::getDisplay() const {
    return display.get();
}

void nwm::Base::setDisplay(Display* dpy){
    display.reset(dpy);
}


void update(Window window, XEvent event, nwm::Base &test){
    window = XCreateSimpleWindow(test.display.get(), test.root, POSITION_X, POSITION_Y,  WIDTH(test.display.get(), SCREEN_NUMBER), HEIGHT(test.display.get(), SCREEN_NUMBER), BORDER ,BlackPixel(test.display.get(), test.screen), BlackPixel(test.display.get(), test.screen));
    XMapWindow(test.display.get(), window);
    while (XNextEvent(test.display.get(), &event) == 0) {
    }
}
void file_check(nwm::Base& test){
    std::string cmd = "touch ";
    cmd = cmd + FILE;
    system(cmd.c_str());
    std::cout << "File created successfully" << std::endl;
}

void init(nwm::Base &test){
    test.config.open(FILE);
    file_check(test);
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Display connection could not be initialized\n";
        std::exit(1);
    }
    test.setDisplay(dpy);
    if (test.display.get() == NULL) {
        std::cerr << "Diplay connection could not be initalized";
    }
    test.screen = DefaultScreen(test.display.get());
    test.root = RootWindow(test.display.get(), test.screen);
}

void clean(nwm::Base &test, Window window){
    XUnmapWindow(test.display.get(), window);
    XDestroyWindow(test.display.get(), window);
    XCloseDisplay(test.display.get());
}

int main(void){
    nwm::De x;
    nwm::Base y;
    init(y);
    update(x.window, x.event, y);
    clean(y,x.window);
    return 0;
}
