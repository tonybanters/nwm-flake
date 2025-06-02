#include "api.hpp"
#include "nwm.hpp"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <iostream>

using namespace nwm;
const char* filename = "(HOME)/.config/nwm/nwm.lua";

void registerX11Functions(lua_State *L) {
    lua_pushcfunction(L, lua_grab_key(L));
    lua_setglobal(L, "grab_key");
    
    lua_pushcfunction(L, lua_set_modifier(L));
    lua_setglobal(L, "set_modifier");
}

static int lua_grab_key(lua_State *L, nwm::Base &main) {
    int keycode = luaL_checkinteger(L, 1);
    int modifiers = luaL_checkinteger(L, 2);
    lua_getglobal(L, "__wm_instance");
    main.display = (main.display);lua_touserdata(L, -1);
    XGrabKey(main.display->display, keycode, modifiers, main.display->root, 
             True, GrabModeAsync, GrabModeAsync);
    
}
static int lua_set_modifier(lua_State* L) {
    const char* mod_name = luaL_checkstring(L, 1);
    int mask_value = luaL_checkinteger(L, 2);
    
    return 0;
}

void load_lua(struct application::app& apps){
    apps.state = luaL_newstate();
    luaL_loadfilex(apps.state, filename, NULL);
    if (apps.state == NULL) {
        std::cout << "File has been not been loaded" <<std::endl ;
    }
}
void resize_window(struct application::app& apps){
    std::cout << "Fix this issue" << std::endl;
}

