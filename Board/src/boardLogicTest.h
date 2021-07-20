#include "chessServer.h"
#include <stdlib.h>

#define MAIN_MENU 0xAAA0
#define FRIENDS 0xAAA1
#define SETTINGS 0xAAA2
#define GAME 0xAAA3

uint32_t mainMenu();
uint32_t game();
uint32_t settings();

