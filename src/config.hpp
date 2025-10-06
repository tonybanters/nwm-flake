#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <X11/keysym.h>
#include "nwm.hpp"

using namespace nwm;
/* appearance */
#define BORDER_WIDTH        2
#define BORDER_COLOR        0x444444
#define FOCUS_COLOR         0xFF5577
#define GAP_SIZE            0

/* key definitions */
#define MODKEY Mod1Mask  // Alt key

/* commands */
static const char *termcmd[]  = { "st",        NULL };
static const char *dmenucmd[] = { "dmenu_run", NULL };
static const char *mastecmd[] = { "/home/xsoder/scripts/master", NULL };

/* keybinds */
static struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(void*, nwm::Base&);
    const char **arg;
} keys[] = {
    /* modifier           key                 function        argument */
    { MODKEY,             XK_Return,          spawn,          termcmd },
    { MODKEY,             XK_d,               spawn,          dmenucmd },
    { MODKEY,             XK_m,               spawn,          mastecmd },
    { MODKEY | ShiftMask, XK_r,               reload_config,  NULL },
    { MODKEY ,            XK_q,               close_window,   NULL },
    { MODKEY,             XK_j,               focus_next,     NULL },
    { MODKEY,             XK_k,               focus_prev,     NULL },
    { MODKEY | ShiftMask, XK_c,               quit_wm,        NULL },
};

#endif // CONFIG_HPP
