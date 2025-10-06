#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <X11/keysym.h>
#include "nwm.hpp"

using namespace nwm;
/* appearance */
#define BORDER_WIDTH        2
#define BORDER_COLOR        0x444444
#define FOCUS_COLOR         0xFF5577
#define GAP_SIZE            10

/* key definitions */
#define MODKEY Mod1Mask  // Alt key

/* commands */
static const char *termcmd[]  = { "st",        NULL };
static const char *dmenucmd[] = { "dmenu_run", NULL };

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
    { MODKEY|ShiftMask,   XK_c,               NULL,           NULL },
    { MODKEY,             XK_j,               NULL,           NULL },
    { MODKEY,             XK_k,               NULL,           NULL },
    { MODKEY,             XK_t,               NULL,           NULL },
    { MODKEY,             XK_q,               NULL,           NULL },
};

#endif // CONFIG_HPP
