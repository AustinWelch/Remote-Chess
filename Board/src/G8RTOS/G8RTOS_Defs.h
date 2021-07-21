#ifndef G8RTOS_DEFS_H
#define G8RTOS_DEFS_H

#include <stdint.h>

#define NULL 0
#define TRUE 1
#define FALSE 0


typedef uint8_t bool_t;

typedef void(*ThreadEntry_g)(void);

#endif
