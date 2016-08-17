#ifndef _VIDEOX_H
#define _VIDEOX_H

#include <stdint.h>
#include <stddef.h>

#define FUNC_MOV 1

void videox_init();
void videox_add_inst(uint32_t val);
uint32_t videox_peak_inst();
void videox_wait();
void videox_exec(int func, const void* src, void* dest, size_t len);
void fast_memcpy(void* dest, const void* src, size_t cnt);

#endif
