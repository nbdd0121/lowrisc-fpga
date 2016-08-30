#ifndef _VIDEOX_H
#define _VIDEOX_H

#include <stdint.h>
#include <stddef.h>

#define FUNC_MOV 0 

void videox_wait();
void videox_exec(int func, const void* src, void* dest, size_t len, int attrib);
void* fast_memcpy(void* dest, const void* src, size_t cnt);

#endif
