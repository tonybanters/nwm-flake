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
    std::exit(1);
}
void todo(char* message, int line) {
    setColor(RED);
    std::cout << message << "At line number" << line << std::endl;
    resetColor();
    std::exit(1);
}
void todo(char* message, char* funtion_name, int line) {
    setColor(RED);
    std::cout << message << "At line number: " << line << std::endl;
    std::cout << "The funtion that needs to be implemented is: " << funtion_name << std::endl;
    resetColor();
    std::exit(1);
}
