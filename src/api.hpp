#ifndef CLIENT_HPP
#define CLIENT_HPP

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace application {
    struct app{
        lua_State *state;
        void *uid;
        int height;
        int width;
    };
}
void resize_window(struct application::app&);
void load_config(struct application::app&);
void load_lua(struct application::app&);
void print(struct application::app&);

#endif //CLIENT_HPP
