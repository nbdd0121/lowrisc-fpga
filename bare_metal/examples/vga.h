#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

#define VIDEO_CR_BASE      0x0
#define VIDEO_CR_BASE_HIGH 0x1
#define VIDEO_CR_DEPTH     0x2
#define VIDEO_CR_ENABLE    0x3
#define VIDEO_CR_POLARITY  0x4
#define VIDEO_CR_PXLFREQ   0x5
#define VIDEO_CR_FB_WIDTH  0x6
#define VIDEO_CR_FB_HEIGHT 0x7
#define VIDEO_CR_FB_BPL    0x8
#define VIDEO_CR_BG_COLOR  0x9

#define VIDEO_CR_H_TOTAL    0x10
#define VIDEO_CR_H_END_DISP 0x11
#define VIDEO_CR_H_SRT_SYNC 0x12
#define VIDEO_CR_H_END_SYNC 0x13
#define VIDEO_CR_V_TOTAL    0x14
#define VIDEO_CR_V_END_DISP 0x15
#define VIDEO_CR_V_SRT_SYNC 0x16
#define VIDEO_CR_V_END_SYNC 0x17

struct vga_mode {
    uint16_t hTotal;
    uint16_t hEndDisp;
    uint16_t hSrtSync;
    uint16_t hEndSync;
    uint16_t vTotal;
    uint16_t vEndDisp;
    uint16_t vSrtSync;
    uint16_t vEndSync;
    uint8_t  pxlFreq;
    uint8_t  polarity;
};

#define VIDEO_MODE_640x480 ((struct vga_mode) {.hTotal = 800, .hEndDisp = 640, .hSrtSync = 656, .hEndSync = 752, .vTotal = 525, .vEndDisp = 480, .vSrtSync = 490, .vEndSync = 492, .pxlFreq = 25, .polarity = 3})
#define VIDEO_MODE_768x576 ((struct vga_mode) {.hTotal = 976, .hEndDisp = 768, .hSrtSync = 792, .hEndSync = 872, .vTotal = 597, .vEndDisp = 576, .vSrtSync = 577, .vEndSync = 580, .pxlFreq = 35, .polarity = 1})
#define VIDEO_MODE_800x600 ((struct vga_mode) {.hTotal = 1056, .hEndDisp = 800, .hSrtSync = 840, .hEndSync = 968, .vTotal = 628, .vEndDisp = 600, .vSrtSync = 601, .vEndSync = 605, .pxlFreq = 40, .polarity = 0})
#define VIDEO_MODE_1024x768 ((struct vga_mode) {.hTotal = 1344, .hEndDisp = 1024, .hSrtSync = 1048, .hEndSync = 1184, .vTotal = 806, .vEndDisp = 768, .vSrtSync = 771, .vEndSync = 777, .pxlFreq = 65, .polarity = 3})
#define VIDEO_MODE_1280x960 ((struct vga_mode) {.hTotal = 1712, .hEndDisp = 1280, .hSrtSync = 1360, .hEndSync = 1496, .vTotal = 994, .vEndDisp = 960, .vSrtSync = 961, .vEndSync = 964, .pxlFreq = 102, .polarity = 2})
#define VIDEO_MODE_1280x1024 ((struct vga_mode) {.hTotal = 1688, .hEndDisp = 1280, .hSrtSync = 1328, .hEndSync = 1440, .vTotal = 1066, .vEndDisp = 1024, .vSrtSync = 1025, .vEndSync = 1028, .pxlFreq = 108, .polarity = 0})

static void video_writeCr(ptrdiff_t loc, uint32_t val) {
    volatile uint32_t *reg = &((volatile uint32_t *)DEV_MAP__io_ext_videoctrl__BASE)[loc];
    *reg = val;
    while(*reg != val);
}

static uint32_t video_readCr(ptrdiff_t loc) {
    volatile uint32_t *reg = &((volatile uint32_t *)DEV_MAP__io_ext_videoctrl__BASE)[loc];
    return *reg;
}

static void video_setEnable(bool en) {
    video_writeCr(VIDEO_CR_ENABLE, en);
}

static void video_switchMode(struct vga_mode mode) {
    video_writeCr(VIDEO_CR_ENABLE, 0);
    video_writeCr(VIDEO_CR_POLARITY, mode.polarity);
    video_writeCr(VIDEO_CR_PXLFREQ, mode.pxlFreq);
    video_writeCr(VIDEO_CR_H_TOTAL, mode.hTotal);
    video_writeCr(VIDEO_CR_H_END_DISP, mode.hEndDisp);
    video_writeCr(VIDEO_CR_H_SRT_SYNC, mode.hSrtSync);
    video_writeCr(VIDEO_CR_H_END_SYNC, mode.hEndSync);
    video_writeCr(VIDEO_CR_V_TOTAL, mode.vTotal);
    video_writeCr(VIDEO_CR_V_END_DISP, mode.vEndDisp);
    video_writeCr(VIDEO_CR_V_SRT_SYNC, mode.vSrtSync);
    video_writeCr(VIDEO_CR_V_END_SYNC, mode.vEndSync);
    video_writeCr(VIDEO_CR_ENABLE, 1);
}
