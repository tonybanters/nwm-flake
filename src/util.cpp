#include "util.hpp"
#include <iostream>

void setColor(int textColor) {
    std::cout << "\033[" << textColor << "m";
}
void resetColor() {
    std::cout << "\033[0m";
}
void todo() {
    setColor(RED);
    std::cout << "Not implemeneted Yet" << std::endl;
    resetColor();
    setColor(RED);
    std::cout << "Exiting" << std::endl;
    resetColor();
    std::exit(1);
}
void error(std::string message) {
    setColor(RED);
    std::cout << message << std::endl;
    resetColor();
    std::exit(1);
}
void todo(char* message) {
    setColor(RED);
    std::cout << message << std::endl;
    resetColor();
    std::exit(1);
}

