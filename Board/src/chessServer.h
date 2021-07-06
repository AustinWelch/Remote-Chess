#include <CC3100SupportPackage/sl_usage/sl_usage.h>
#include "BSP.h"
#include "debug.h"
#include "fpu.h"
#include "rom.h"
#include "sysctl.h"
#include <stdio.h>
#include "application_commands.h"
#include <string.h>

#define WEBPAGE "3.95.241.253"
#define BOARD_ID 0

void chessServer_init();
uint8_t chessServer_makeMove(char* gameCode, char* move);