#include "nwm.hpp"
int main(void){
    nwm::De x;
    nwm::UI_elt y;
    init(y);
    update(x.window, x.event, y);
    clean(y,x.window);
    return 0;
}
