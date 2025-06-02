#include "api.hpp"
#include "util.hpp"
#include <lauxlib.h>
#include <lualib.h>

const char* filename = "(HOME)/.config/nwm/nwm.lua";

void resize_window(struct application::app&){
    todo();
}

void load_config(struct application::app&){
    todo();
}
void load_lua(struct application::app& apps){
    apps.state = luaL_newstate();
    luaL_loadfilex(apps.state, filename, NULL);
}
void print(struct application::app&){
    todo();
}
