#include "nwm.hpp"
#include "api.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <sys/stat.h>
#include <cassert>
#include <cstdlib>
#include <iostream>


void update(nwm::De &env, nwm::Base &test, application::app &apps){
        env.window = XCreateSimpleWindow(test.display, test.root, POSITION_X, POSITION_Y,  WIDTH(test.display, SCREEN_NUMBER), HEIGHT(test.display, SCREEN_NUMBER), BORDER ,BlackPixel(test.display, test.screen), BlackPixel(test.display, test.screen));
        XMapWindow(test.display, env.window);
        load_lua(apps);
        while (XNextEvent(test.display, env.event) == 0) {
            // system("TERMINAL");
    }
}
void file_check(nwm::Base& test){
    std::string cmd = "touch ";
    std::string config = "~/.config/nwm/nwm.lua ";
    cmd = cmd + config;
    system(cmd.c_str());
    std::cout << "File created successfully" << std::endl;
}

void init(nwm::Base &test){
    test.display = XOpenDisplay(NULL);
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
    setenv("DISPLAY", ":0", 1);
    char* display = getenv("DISPLAY");
    std::cout << "DISPLAY inside WM: " << (display ? display : "not set") << std::endl;
    application::app a;
    nwm::De x;
    nwm::Base y;
    init(y);
    update(x,y,a);
    clean(y,x);
    return 0;
}
