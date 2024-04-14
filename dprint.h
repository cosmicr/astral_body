#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdarg.h>
#include <stdio.h>

//#define DEBUG

#ifdef DEBUG
#define dprintf(format) printf(format)
#define dprintf_args(format, ...) printf(format, __VA_ARGS__)
#else
#define dprintf(format)
#define dprintf_args(format, ...)
#endif

#endif // DEBUG_PRINT_H