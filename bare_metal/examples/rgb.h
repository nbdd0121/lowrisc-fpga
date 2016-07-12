#include <stdint.h>

struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static inline struct rgb rgb32_unpack(uint32_t c) {
    struct rgb ret = {
        .r = c >> 16,
        .g = c >> 8,
        .b = c
    };
    return ret;
}

static inline uint32_t rgb32_pack(struct rgb c) {
    return (c.r << 16) | (c.g << 8) | c.b;
}

static inline struct rgb rgb16_unpack(uint16_t c) {
    struct rgb ret = {
        .r = (c >> 11) & 31,
        .g = (c >> 5) & 63,
        .b = c & 31
    };
    ret.r = (ret.r << 3) | (ret.r >> 2);
    ret.g = (ret.g << 2) | (ret.g >> 4);
    ret.b = (ret.b << 3) | (ret.b >> 2);
    return ret;
}

static inline uint32_t rgb16_pack(struct rgb c) {
    return (c.r >> 3 << 11) | (c.g >> 2 << 5) | (c.b >> 3);
}

static inline uint32_t rgb16_to32(uint16_t c) {
    return rgb32_pack(rgb16_unpack(c));
}

static inline uint16_t rgb32_to16(uint32_t c) {
    return rgb16_pack(rgb32_unpack(c));
}

static inline struct rgb rgb8_unpack(uint16_t c) {
    uint8_t r = (c >> 5) & 7;
    uint8_t g = (c >> 2) & 7;
    uint8_t b = c & 3;
    struct rgb ret = {
        .r = (r << 5) | (r << 2) | (r >> 1),
        .g = (g << 5) | (g << 2) | (g >> 1),
        .b = (b << 6) | (b << 4) | (b << 2) | b
    };
    return ret;
}

static inline uint32_t rgb8_pack(struct rgb c) {
    return (c.r >> 5 << 5) | (c.g >> 5 << 2) | (c.b >> 6);
}

static inline uint32_t rgb8_to32(uint16_t c) {
    return rgb32_pack(rgb8_unpack(c));
}

static inline uint16_t rgb32_to8(uint32_t c) {
    return rgb8_pack(rgb32_unpack(c));
}

static inline struct rgb grayscale_unpack(uint8_t c) {
    uint8_t comp = (c << 4) | c;
    struct rgb ret = {
        .r = comp,
        .g = comp,
        .b = comp
    };
    return ret;
}

static inline uint32_t grayscale_pack(struct rgb c) {
    return ((c.r >> 2) + (c.g >> 1) + (c.b >> 2)) >> 4;
}

static inline uint32_t grayscale_to32(uint16_t c) {
    return rgb32_pack(grayscale_unpack(c));
}

static inline uint16_t rgb32_tograyscale(uint32_t c) {
    return grayscale_pack(rgb32_unpack(c));
}



