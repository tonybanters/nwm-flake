#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <X11/keysym.h>
#include "nwm.hpp"

using namespace nwm;

#define BORDER_WIDTH        3
#define BORDER_COLOR        0x181818
#define FOCUS_COLOR         0x005577
#define GAP_SIZE            2

#define BAR_POSITION        0
#define SCROLL_WINDOWS_VISIBLE 2

#define FONT                "Iosevka:size=10"

static const std::vector<std::string> WIDGET = {
    "1","2","3","4","5","6","7","8","9"
};

#define RESIZE_STEP         60

#define SCROLL_STEP         550

#define MODKEY Mod4Mask

static const char *termcmd[]    = { "st",        NULL };
static const char *emacs[]      = { "emacs",     NULL };
static const char *dmenucmd[]   = { "dmenu_run", NULL };
static const char *browser[]    = { "firefox",   NULL };

static const int ws0 = 0;
static const int ws1 = 1;
static const int ws2 = 2;
static const int ws3 = 3;
static const int ws4 = 4;
static const int ws5 = 5;
static const int ws6 = 6;
static const int ws7 = 7;
static const int ws8 = 8;

static const int mon0 = 0;
static const int mon1 = 1;
static const int mon2 = 2;

static const int scroll_visible_1 = 1;
static const int scroll_visible_2 = 2;
static const int scroll_visible_3 = 3;
static const int scroll_visible_4 = 4;
static const int scroll_visible_5 = 5;

static struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(void*, nwm::Base&);
    const void *arg;
} keys[] = {
    { MODKEY,             XK_Return,          spawn,          termcmd },
    { MODKEY,             XK_d,               spawn,          dmenucmd },
    { MODKEY,             XK_c,               spawn,          emacs },
    { MODKEY,             XK_b,               spawn,          browser },
    { MODKEY,             XK_r,               toggle_bar,     NULL },
    { MODKEY,             XK_q,               close_window,   NULL },

    { MODKEY,             XK_a,               toggle_gap,     NULL },
    { MODKEY,             XK_t,               toggle_layout,  NULL },
    { MODKEY,             XK_f,               toggle_fullscreen, NULL },
    { MODKEY|ShiftMask,   XK_f,               toggle_scroll_maximize, NULL },
    { MODKEY|ShiftMask,   XK_space,           toggle_float,   NULL },

    { MODKEY,             XK_j,               focus_next,     NULL },
    { MODKEY,             XK_k,               focus_prev,     NULL },
    { MODKEY | ShiftMask, XK_h,               swap_prev,      NULL },
    { MODKEY | ShiftMask, XK_l,               swap_next,      NULL },

    { MODKEY,             XK_h,               resize_master,  (void*)-RESIZE_STEP },
    { MODKEY,             XK_l,               resize_master,  (void*)RESIZE_STEP },

    { MODKEY,             XK_Left,            scroll_left,    NULL },
    { MODKEY,             XK_Right,           scroll_right,   NULL },

    { MODKEY,             XK_comma,           focus_monitor,  (void*)&mon0 },
    { MODKEY,             XK_period,          focus_monitor,  (void*)&mon1 },
    { MODKEY,             XK_slash,           focus_monitor,  (void*)&mon2 },

    { MODKEY|ShiftMask,   XK_comma,           set_scroll_visible, (void*)&scroll_visible_2 },
    { MODKEY|ShiftMask,   XK_period,          set_scroll_visible, (void*)&scroll_visible_3 },
    { MODKEY|ShiftMask,   XK_slash,           set_scroll_visible, (void*)&scroll_visible_4 },

    { MODKEY,             XK_equal,          increment_scroll_visible, NULL },
    { MODKEY,             XK_minus,          decrement_scroll_visible, NULL },

    { MODKEY,             XK_1,               switch_workspace, (void*)&ws0 },
    { MODKEY,             XK_2,               switch_workspace, (void*)&ws1 },
    { MODKEY,             XK_3,               switch_workspace, (void*)&ws2 },
    { MODKEY,             XK_4,               switch_workspace, (void*)&ws3 },
    { MODKEY,             XK_5,               switch_workspace, (void*)&ws4 },
    { MODKEY,             XK_6,               switch_workspace, (void*)&ws5 },
    { MODKEY,             XK_7,               switch_workspace, (void*)&ws6 },
    { MODKEY,             XK_8,               switch_workspace, (void*)&ws7 },
    { MODKEY,             XK_9,               switch_workspace, (void*)&ws8 },

    { MODKEY | ShiftMask, XK_1,               move_to_workspace, (void*)&ws0 },
    { MODKEY | ShiftMask, XK_2,               move_to_workspace, (void*)&ws1 },
    { MODKEY | ShiftMask, XK_3,               move_to_workspace, (void*)&ws2 },
    { MODKEY | ShiftMask, XK_4,               move_to_workspace, (void*)&ws3 },
    { MODKEY | ShiftMask, XK_5,               move_to_workspace, (void*)&ws4 },
    { MODKEY | ShiftMask, XK_6,               move_to_workspace, (void*)&ws5 },
    { MODKEY | ShiftMask, XK_7,               move_to_workspace, (void*)&ws6 },
    { MODKEY | ShiftMask, XK_8,               move_to_workspace, (void*)&ws7 },
    { MODKEY | ShiftMask, XK_9,               move_to_workspace, (void*)&ws8 },

    { MODKEY | ShiftMask, XK_q,               quit_wm,        NULL },
};

#endif //CONFIG_HPP
