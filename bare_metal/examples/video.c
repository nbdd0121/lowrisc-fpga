#include <stdio.h>
#include <string.h>
#include "diskio.h"
#include "ff.h"
#include "uart.h"
#include "memory.h"

#include "rgb.h"
#include "vga.h"
#include "encoding.h"

#undef putchar

#define DEPTH_32 0
#define DEPTH_16 1
#define DEPTH_8  2
#define DEPTH_GRAYSCALE 3

/* File reading routine */
int readFile(const char* name, void* dest) {
    FATFS FatFs;

    if(f_mount(&FatFs, "", 1)) {
        printf("Fail to mount SD driver!\n");
        return -1;
    }

    FIL fil;                /* File object */
    uint8_t* buffer = dest; /* File copy buffer */
    FRESULT fr;             /* FatFs return code */
    uint32_t br;            /* Read count */
    uint32_t i;

    fr = f_open(&fil, name, FA_READ);
    if (fr) {
        printf("Failed to open %s!\n", name);
        return -1;
    }

    uint32_t fsize = 0;
    for (;;) {
        fr = f_read(&fil, &buffer[fsize], 64, &br);  /* Read a chunk of source file */
        if (fr || br == 0) break; /* error or eof */
        fsize += br;
    }

    if(f_close(&fil)) {
        printf("Fail to close file!");
        return -1;
    }

    if(f_mount(NULL, "", 1)) {         /* unmount it */
        printf("fail to umount disk!");
        return 1;
    }
}

/* Fill the whole framebuffer */
void fillColor(volatile uint32_t* mem, uint32_t color) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            mem[idx] = color;
        }
    }
}

/* Fill the framebuffer using black&white grids */
void fillGrid(volatile uint32_t* mem) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            mem[idx] = (x + y) & 1 ? 0xFFFFFF : 0;
        }
    }
}

void changeColor(uint32_t color) {
    static uint32_t prev = 0xFFFFFFFF;
    if (color == prev) return;
    prev = color;
    if (color == 0xFFFFFFFF)
        printf("\x1b[0m");
    else
        printf("\x1b[48;2;%d;%d;%dm", (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

/* Timer */

uint64_t readCycle();

static uint64_t targetTime;

void startTiming(uint64_t cnt) {
    uint64_t start = readCycle();
    targetTime = start + cnt * 25000;
}

bool isTimeOver() {
    return readCycle() > targetTime;
}

void dump(volatile uint32_t* mem) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            changeColor(mem[idx]);
            putchar(' ');
        }
        changeColor(0xFFFFFFFF);
        putchar('\n');
    }
}

void copyImg(volatile uint32_t* from, volatile uint32_t* to) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            to[idx] = from[idx];
        }
    }
}

void copyImg16(volatile uint32_t* from, volatile uint16_t* to) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            to[idx] = rgb32_to16(from[idx]);
        }
    }
}

void copyImg8(volatile uint32_t* from, volatile uint8_t* to) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            int idx = (y << 7) | x;
            to[idx] = rgb32_to8(from[idx]);
        }
    }
}

void copyImgGrayscale(volatile uint32_t* from, volatile uint8_t* to) {
    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x += 2) {
            int idx = (y << 7) | x;
            to[idx >> 1] = (rgb32_tograyscale(from[idx + 1]) << 4) | rgb32_tograyscale(from[idx]);
        }
    }
}

#define SWAP(a, b) do{typeof(a) tmp = a; a = b; b = tmp;}while(0)

int main() {
    uart_init();
    printf("----------\n");

    uint32_t colors[] = {0x000000, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};

    uint32_t* ddr_base = (uint32_t*)(void*)get_ddr_base();

    volatile uint32_t *mem = ddr_base + 0x8000;

    readFile("img.raw", ddr_base);
    readFile("img2.raw", ddr_base + 0x4000);

    printf("Initializing...\n");
    video_writeCr(VIDEO_CR_ENABLE, 0);
    printf("Display disabled...\n");

    uint32_t mode = 0;
    uint32_t step;

    if(mode == DEPTH_32) {
        copyImg(ddr_base, mem);
        copyImg(ddr_base + 0x4000, mem + 0x4000);
        copyImg(ddr_base, mem + 0x8000);
        step = 128 * 4;
    }else if(mode==DEPTH_16){
        copyImg16(ddr_base, (volatile uint16_t*)mem);
        copyImg16(ddr_base + 0x4000, (volatile uint16_t*)mem + 0x4000);
        copyImg16(ddr_base, (volatile uint16_t*)mem + 0x8000);
        step = 128 * 2;
    }else if(mode==DEPTH_8){
        copyImg8(ddr_base, (volatile uint8_t*)mem);
        copyImg8(ddr_base + 0x4000, (volatile uint8_t*)mem + 0x4000);
        copyImg8(ddr_base, (volatile uint8_t*)mem + 0x8000);
        step = 128;
    }else{
        copyImgGrayscale(ddr_base, (volatile uint8_t*)mem);
        copyImgGrayscale(ddr_base + 0x4000, (volatile uint8_t*)mem + 0x2000);
        copyImgGrayscale(ddr_base, (volatile uint8_t*)mem + 0x4000);
        step = 128 / 2;
    }

    printf("Framebuffer updated\n");
    video_writeCr(VIDEO_CR_DEPTH, mode);
    video_writeCr(VIDEO_CR_FB_WIDTH, 128);
    video_writeCr(VIDEO_CR_FB_HEIGHT, 128);
    video_writeCr(VIDEO_CR_FB_BPL, step);
    video_writeCr(VIDEO_CR_BG_COLOR, 0xFFFFFF);
    video_writeCr(VIDEO_CR_BASE, (uint32_t)(uintptr_t)mem);
    video_writeCr(VIDEO_CR_BASE_HIGH, (uint32_t)((uintptr_t)mem >> 32));
    video_writeCr(VIDEO_CR_ENABLE, 1);
    printf("Resolution changed\n");

    char buffer[16];
    int bufptr = 0;

    targetTime = 0;

    uint32_t s = 0;
    uint32_t mask = step * 128 - 1;
    uint32_t mask2 = step * 256 - 1;
    while (true) {
        if (isTimeOver()) {
            s += step;
            if ((s&mask) == 0) {
                startTiming(1010);
            } else {
                startTiming(10);
            }
            video_writeCr(VIDEO_CR_BASE, (uint32_t)(uintptr_t)mem + (s & mask2));
        }
        if (uart_check_read_irq()) {
            char c = uart_recv();
            switch (c) {
                case 127:
                    if (bufptr > 0) {
                        bufptr--;
                        printf("\b \b");
                    }
                    break;
                case 13:
                    putchar('\n');
                    buffer[bufptr] = 0;
                    if (strcmp(buffer, "1024") == 0) {
                        video_switchMode(VIDEO_MODE_1024x768);
                        printf("Resolution switched to 1024x768\n");
                    } else if (strcmp(buffer, "800") == 0) {
                        video_switchMode(VIDEO_MODE_800x600);
                        printf("Resolution switched to 800x600\n");
                    } else if (strcmp(buffer, "768") == 0) {
                        video_switchMode(VIDEO_MODE_768x576);
                        printf("Resolution switched to 768x576\n");
                    } else if (strcmp(buffer, "1280") == 0) {
                        video_switchMode(VIDEO_MODE_1280x960);
                        printf("Resolution switched to 1280x960\n");
                    } else if (strcmp(buffer, "640") == 0) {
                        video_switchMode(VIDEO_MODE_640x480);
                        printf("Resolution switched to 640x480\n");
                    } else if (strcmp(buffer, "1280x1024") == 0) {
                        video_switchMode(VIDEO_MODE_1280x1024);
                        printf("Resolution switched to 1280x1024\n");
                    } else {
                        printf("Unknown command %s\n", buffer);
                    }
                    bufptr = 0;
                    break;
                default:
                    if (bufptr < 15) {
                        buffer[bufptr++] = c;
                        putchar(c);
                    }
            }
        }
    }

    return 0;
}

