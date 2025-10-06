#ifndef CONFIG_H
#define CONFIG_H

#include <X11/keysym.h>

/* appearance */
#define BORDER_WIDTH        2
#define BORDER_COLOR        0x444444
#define FOCUS_COLOR         0xFF5577
#define GAP_SIZE            10

/* key definitions */
#define MODKEY Mod4Mask  // Alt key

/* commands */
static const char *termcmd[]  = { "st", "-f", "monospace:size=10", NULL };
static const char *dmenucmd[] = { "dmenu_run", "-fn", "monospace-10", NULL };

/* keybinds */
static struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(void*);
    const char **arg;
} keys[] = {
    /* modifier           key                 function        argument */
    { MODKEY,             XK_Return,          NULL,           termcmd },
    { MODKEY,             XK_d,               NULL,           dmenucmd },
    { MODKEY|ShiftMask,   XK_c,               NULL,           NULL },
    { MODKEY,             XK_j,               NULL,           NULL },
    { MODKEY,             XK_k,               NULL,           NULL },
    { MODKEY,             XK_t,               NULL,           NULL },
    { MODKEY,             XK_q,               NULL,           NULL },
};

#endif // CONFIG_H
