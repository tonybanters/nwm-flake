#include "nwm.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <sys/stat.h>
#include <cassert>
#include <cstdlib>
#include <iostream>


void update(nwm::De &env, nwm::Base &test){
    env.window = XCreateSimpleWindow(test.display, test.root, POSITION_X, POSITION_Y,  WIDTH(test.display, SCREEN_NUMBER), HEIGHT(test.display, SCREEN_NUMBER), BORDER ,BlackPixel(test.display, test.screen), BlackPixel(test.display, test.screen));
    XMapWindow(test.display, env.window);
    while (XNextEvent(test.display, env.event) == 0) {
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
    if (test.display == NULL) {
        std::cerr << "Display connection could not be initialized\n";
        std::exit(1);
    }
    test.screen = DefaultScreen(test.display);
    test.root = RootWindow(test.display, test.screen);
}

void clean(nwm::Base &test, nwm::De &env){
    XUnmapWindow(test.display, env.window);
    XDestroyWindow(test.display, env.window);
    XCloseDisplay(test.display);
}

int main(void){
    nwm::De x;
    nwm::Base y;
    init(y);
    update(x,y);
    clean(y,x);
    return 0;
}
