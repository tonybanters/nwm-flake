#ifndef TILING_HPP
#define TILING_HPP

#include "nwm.hpp"

namespace nwm {

void tile_windows(Base &base);
void tile_horizontal(Base &base);

void resize_master(void *arg, Base &base);

void scroll_left(void *arg, Base &base);
void scroll_right(void *arg, Base &base);

void toggle_layout(void *arg, Base &base);

void swap_next(void *arg, Base &base);
void swap_prev(void *arg, Base &base);

}

#endif // TILING_HPP
