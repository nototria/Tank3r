#include <curses.h>
#include <locale.h>
#include <string>
#include <unistd.h>

// Tank Unicode representations
const std::string DOWN_R1 = " ■   ■ ";
const std::string DOWN_R2 = "  ▓▓▓  ";
const std::string DOWN_R3 = " ■═╦═■ ";

const std::string TOP_R1 = " ■═╧═■ ";
const std::string TOP_R2 = "  ▓▓▓  ";
const std::string TOP_R3 = " ■   ■ ";

const std::string RIGHT_R1 = " ■   ■ ";
const std::string RIGHT_R2 = "  ▓▓▓╠═";
const std::string RIGHT_R3 = " ■   ■ ";

const std::string LEFT_R1 = " ■   ■ ";
const std::string LEFT_R2 = "═╣▓▓▓  ";
const std::string LEFT_R3 = " ■   ■ ";

const int ANIMATION_DELAY = 60000;
