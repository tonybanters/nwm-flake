#!/bin/bash

Xephyr -screen 1280x720 -ac :1 &
XEPHYR_PID=$!

sleep 1

export DISPLAY=:1
xset +fp /usr/share/fonts/misc
xset +fp /usr/share/fonts/75dpi
xset +fp /usr/share/fonts/100dpi
xset fp rehash
xset q | grep "Font Path"

./nwm &
WM_PID=$!
sleep 2
feh --bg-fill /home/xsoder/wallpaper/master.png &
xclock &
sleep 2
st &
echo "Window manager is running in Xephyr"
echo "Press Ctrl+C to stop"
echo ""
echo "Key bindings:"
echo "  Alt+Return  - Open terminal"
echo "  Alt+j       - Focus next window"
echo "  Alt+k       - Focus previous window"
echo "  Alt+t       - Tile windows"
echo "  Alt+Shift+c - Close focused window"
echo "  Alt+Shift+q - Quit window manager"

trap "kill $WM_PID $XEPHYR_PID 2>/dev/null" EXIT
wait $WM_PID
