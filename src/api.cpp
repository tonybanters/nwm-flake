#include "api.hpp"
#include "util.hpp"
#include <lauxlib.h>
#include <lualib.h>
#include <iostream>

const char* filename = "(HOME)/.config/nwm/nwm.lua";

void resize_window(struct application::app& apps){
    todo("Implement the resize ability", "resize_window" ,10);
}

void load_config(struct application::app& apps){
    todo("Load the config file", "load_config" ,14);
}
void load_lua(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
    apps.state = luaL_newstate();
    luaL_loadfilex(apps.state, filename, NULL);
    todo("Lua loading and parsing", 20);
}
void print(struct application::app& apps){
    todo("Implement the resize ability", "resize_window" ,23);
}
