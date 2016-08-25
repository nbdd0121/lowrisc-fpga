#include <stdio.h>
#include <string.h>
#include <dev_map.h>
#include <videox.h>

#define OFFSET_LIMIT (1<<13)
#define FUNC_MAX 9

void videox_init() {
}

static void videox_write_src(uint64_t val) {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[2] == 128);
    reg[2] = (uint32_t)val;
    reg[3] = (uint32_t)(val >> 32);
}

static void videox_write_dest(uint64_t val) {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[4] == 128);
    reg[4] = (uint32_t)val;
    reg[5] = (uint32_t)(val >> 32);
}

static void videox_write_inst(uint32_t val) {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[0] == 128);
    reg[0] = val;
}

void videox_wait() {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[0] != 0);
}

void videox_exec(int func, const void* src, void* dest, size_t len) {
    if (func < FUNC_MOV || func > FUNC_MAX) {
        printf("Invalid function number %d", func);
        return;
    }
    uintptr_t src_ptr = (uintptr_t)src;
    uintptr_t dest_ptr = (uintptr_t)dest;
    if ((src_ptr & 63) != 0 || (dest_ptr & 63) != 0 || (len & 63) != 0) {
        printf("Unaligned access");
        return;
    }

    videox_write_src((uintptr_t)src << 21 | len << 5 | 1);
    videox_write_dest((uintptr_t)dest << 21 | len << 5 | 1);
    videox_write_inst(func);
}

void* fast_memcpy(void* dest, const void* src, size_t cnt) {
    void* dest0 = dest;

    // Not worth doing that
    if (cnt <= 128) {
        memcpy(dest, src, cnt);
        return dest0;
    }
    // Sorry, can't really do that
    size_t diffcnt = (uintptr_t)dest & 63;
    if (diffcnt != ((uintptr_t)src & 63)) {
        memcpy(dest, src, cnt);
        return dest0;
    }
    // Make it aligned first
    if (diffcnt != 0) {
        memcpy(dest, src, 64 - diffcnt);
        dest += 64 - diffcnt;
        src += 64 - diffcnt;
        cnt -= 64 - diffcnt;
    }

    videox_write_inst(8);

    while (cnt >= 64) {
        size_t transcnt = cnt > 0x40000 ? 0x40000 : cnt &~ 63;
        int last = cnt <= 0x40000;
        videox_write_src((uintptr_t)src << 21 | transcnt << 5 | last);
        videox_write_dest((uintptr_t)dest << 21 | transcnt << 5 | last);
        src += transcnt;
        dest += transcnt;
        cnt -= transcnt;
    }

    if (cnt != 0) {
        memcpy(dest, src, cnt);
    }

    videox_wait();

    return dest0;
}
