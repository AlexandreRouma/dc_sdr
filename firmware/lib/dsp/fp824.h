#pragma once
#include <stdint.h>

typedef int32_t fp824_t;

#define FP_CONST(val)   ((fp824_t)((val)*(1 << 24)))
#define FP_MUL(a, b)    (((a) >> 12)*((b) >> 12))