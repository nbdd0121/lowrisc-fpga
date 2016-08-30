#include <stdio.h>
#include <string.h>
#include <dev_map.h>
#include <videox.h>

#define OFFSET_LIMIT (1<<13)
#define FUNC_MAX 2

static void videox_write_src(uint64_t val) {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[0] == 128);
    reg[0] = (uint32_t)val;
    reg[1] = (uint32_t)(val >> 32);
}

static void videox_write_dest(uint64_t val) {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[2] == 128);
    reg[2] = (uint32_t)val;
    reg[3] = (uint32_t)(val >> 32);
}

void videox_wait() {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    while(reg[2] != 0);
}

void videox_exec(int func, const void* src, void* dest, size_t len, int attrib) {
    if (func > FUNC_MAX) {
        printf("Invalid function number %d", func);
        return;
    }
    uintptr_t src_ptr = (uintptr_t)src;
    uintptr_t dest_ptr = (uintptr_t)dest;
    if ((src_ptr & 63) != 0 || (dest_ptr & 63) != 0 || (len & 63) != 0) {
        printf("Unaligned access");
        return;
    }

    videox_write_src((uintptr_t)src | ((uintptr_t)len << 34) | (1ULL << 55) | func | ((attrib & 63ull) << 56));
    videox_write_dest((uintptr_t)dest | ((uintptr_t)len << 34));
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

    while (cnt >= 64) {
        size_t transcnt = cnt > 0x40000 ? 0x40000 : cnt &~ 63;
        int last = cnt <= 0x40000;
        videox_write_src((uintptr_t)src | ((uintptr_t)transcnt << 34) | ((uintptr_t)last << 55));
        videox_write_dest((uintptr_t)dest | ((uintptr_t)transcnt << 34));
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
