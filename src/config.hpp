#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <X11/keysym.h>
#include "nwm.hpp"

using namespace nwm;

/* appearance */
#define BORDER_WIDTH        3
#define BORDER_COLOR        0x181818
#define FOCUS_COLOR         0xFF5577
#define GAP_SIZE            6

/* font */
#define FONT                "IosevkaNerdFont:size=12"

/* resize step size (in pixels) */
#define RESIZE_STEP         40

/* horizontal scroll step (in pixels) */
#define SCROLL_STEP         800

/* key definitions */
#define MODKEY Mod4Mask  // Win key

/* commands */
static const char *termcmd[]  = { "st",        NULL };
static const char *emacs[]    = { "emacs",     NULL };
static const char *dmenucmd[] = { "dmenu_run", NULL };
static const char *mastecmd[] = { "/home/xsoder/scripts/master", NULL };

/* workspace arguments */
static const int ws0 = 0;
static const int ws1 = 1;
static const int ws2 = 2;
static const int ws3 = 3;
static const int ws4 = 4;
static const int ws5 = 5;
static const int ws6 = 6;
static const int ws7 = 7;
static const int ws8 = 8;

/* keybinds */
static struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(void*, nwm::Base&);
    const void *arg;
} keys[] = {
    /* modifier           key                 function        argument */
    { MODKEY,             XK_Return,          spawn,          termcmd },
    { MODKEY,             XK_d,               spawn,          dmenucmd },
    { MODKEY,             XK_m,               spawn,          mastecmd },
    { MODKEY,             XK_c,               spawn,          emacs },
    { MODKEY | ShiftMask, XK_r,               reload_config,  NULL },
    { MODKEY,             XK_q,               close_window,   NULL },
    { MODKEY,             XK_a,               toggle_gap,     NULL },
    { MODKEY,             XK_t,               toggle_layout,  NULL },
    
    /* Focus and movement */
    { MODKEY,             XK_j,               focus_next,     NULL },
    { MODKEY,             XK_k,               focus_prev,     NULL },
    { MODKEY | ShiftMask, XK_h,               swap_prev,      NULL },
    { MODKEY | ShiftMask, XK_l,               swap_next,      NULL },
    
    /* Master resize (vertical mode) */
    { MODKEY,             XK_h,               resize_master,  (void*)-RESIZE_STEP },
    { MODKEY,             XK_l,               resize_master,  (void*)RESIZE_STEP },
    
    /* Horizontal scrolling (horizontal mode) */
    { MODKEY,             XK_Left,            scroll_left,    NULL },
    { MODKEY,             XK_Right,           scroll_right,   NULL },
    
    /* Workspace switching */
    { MODKEY,             XK_1,               switch_workspace, (void*)&ws0 },
    { MODKEY,             XK_2,               switch_workspace, (void*)&ws1 },
    { MODKEY,             XK_3,               switch_workspace, (void*)&ws2 },
    { MODKEY,             XK_4,               switch_workspace, (void*)&ws3 },
    { MODKEY,             XK_5,               switch_workspace, (void*)&ws4 },
    { MODKEY,             XK_6,               switch_workspace, (void*)&ws5 },
    { MODKEY,             XK_7,               switch_workspace, (void*)&ws6 },
    { MODKEY,             XK_8,               switch_workspace, (void*)&ws7 },
    { MODKEY,             XK_9,               switch_workspace, (void*)&ws8 },
    
    /* Move window to workspace */
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

#endif // CONFIG_HPP
