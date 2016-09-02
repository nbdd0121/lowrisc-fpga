#ifndef _VIDEOX_H
#define _VIDEOX_H

#include <stdint.h>
#include <stddef.h>

#define FUNC_MOV 0
#define FUNC_YUV422TO444 1
#define FUNC_YUV444TORGB 2
#define FUNC_RGB32TO16 3
#define FUNC_SATURATE 4
#define FUNC_IDCT 5

void videox_wait();
void videox_exec(int func, const void* src, void* dest, size_t len, int attrib);
void* fast_memcpy(void* dest, const void* src, size_t cnt);

#endif
