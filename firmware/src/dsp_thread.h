#pragma once
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sem.h"

extern semaphore_t bufferFull;
extern semaphore_t bufferEmpty;
extern uint16_t* buffer;
extern int bufferSize;
extern int bufferId;

void dsp_init();
void dsp_thread();