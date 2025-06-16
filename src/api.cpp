#include "api.hpp"
#include "util.hpp"
#include <cstdio>
#include <iostream>
#include <lauxlib.h>
#include <lualib.h>
#include <string>

const char *filename = "(HOME)/programming/nwm/config/nwm.lua";

void resize_window(struct application::app &apps) {
  std::string message = "Implement the resize ability";
  std::string func_name = "resize_window";
  todo(message, func_name, 10);
}

void load_config(struct application::app &apps) {
  std::string message = "Loading the config file not implemenated";
  std::string func_name = "load_config";
  todo(message, func_name, 18);
}

void load_lua(struct application::app &apps) {
  apps.state = luaL_newstate();
  luaL_loadfilex(apps.state, filename, NULL);
  std::string message = "Lua loading and parsing";
  todo(message, 20);
}

void print(struct application::app &apps) {
  std::string message = "Implement the resize ability";
  std::string func_name = "resize_window";
  todo(message, func_name, 20);
}
