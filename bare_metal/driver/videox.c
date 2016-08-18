#include <stdio.h>
#include <string.h>
#include <dev_map.h>
#include <videox.h>

#define OFFSET_LIMIT (1<<13)
#define FUNC_MAX 9

static const void* videox_src_base;
static void* videox_dest_base;
static int issue_cnt;

void videox_init() {
    videox_src_base  = NULL;
    videox_dest_base = NULL;
    issue_cnt = 0;
}

void videox_add_inst(uint32_t val) {
    if (issue_cnt == 32) videox_wait();
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    *reg = val;
    issue_cnt++;
}

uint32_t videox_peak_inst() {
    volatile uint32_t *reg = (volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE;
    return *reg;
}

void videox_wait() {
    while(videox_peak_inst() != 0);
    issue_cnt = 0;
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

    if (src < videox_src_base || src - videox_src_base >= OFFSET_LIMIT) {
        if ((uint32_t)(src_ptr >> 32) != (uint32_t)((uintptr_t)videox_src_base >> 32)) {
            videox_add_inst(2 | (uint32_t)src_ptr);
            videox_add_inst((uint32_t)(src_ptr >> 32));
        } else {
            videox_add_inst(4 | (uint32_t)src_ptr);
        }
        videox_src_base = src;
    }

    if (dest < videox_dest_base || dest - videox_dest_base >= OFFSET_LIMIT) {
        if ((uint32_t)(dest_ptr >> 32) != (uint32_t)((uintptr_t)videox_dest_base >> 32)) {
            videox_add_inst(3 | (uint32_t)dest_ptr);
            videox_add_inst((uint32_t)(dest_ptr >> 32));
        } else {
            videox_add_inst(5 | (uint32_t)dest_ptr);
        }
        videox_dest_base = dest;
    }

    videox_add_inst((len << 14) | (dest - videox_dest_base) << 7 | (src - videox_src_base) | func);
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
        size_t transcnt = cnt >= 8128 ? 8128 : cnt &~ 63;
        videox_exec(FUNC_MOV, src, dest, transcnt);
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
