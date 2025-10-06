#include "util.hpp"
#include <iostream>

void setColor(int textColor) { 
    std::cout << "\033[" << textColor << "m"; 
}

void resetColor() { 
    std::cout << "\033[0m"; 
}
