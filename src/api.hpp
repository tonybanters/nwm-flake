#ifndef CLIENT_HPP
#define CLIENT_HPP

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace application {
struct app {
  lua_State *state;
  void *uid;
  int height;
  int width;
};
} // namespace application

namespace nwm {
struct Base;
}

void resize_window(struct application::app &);
void load_config(struct application::app &);
void load_lua(struct application::app &);
void registerX11Functions(lua_State *L);

int lua_grab_key(lua_State *L);
int lua_set_modifier(lua_State *L);
int lua_keysym_to_keycode(lua_State *L);

#endif // CLIENT_HPP
