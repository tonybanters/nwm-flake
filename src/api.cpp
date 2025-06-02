#include "api.hpp"
#include "util.hpp"
#include <lauxlib.h>
#include <lualib.h>
#include <iostream>

const char* filename = "(HOME)/.config/nwm/nwm.lua";

void resize_window(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
}

void load_config(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
    //todo();
}
void load_lua(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
    apps.state = luaL_newstate();
    luaL_loadfilex(apps.state, filename, NULL);
}
void print(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
    apps.state = luaL_newstate();
    luaL_loadfilex(apps.state, filename, NULL);
    //todo();
}
