#include <stdio.h>
#include "uart.h"
#include "memory.h"
#include "encoding.h"
#include "videox.h"

int main() {
    uart_init();
    videox_init();
    uint64_t* ddr_base_src  = (uint64_t*)(void*)get_ddr_base();
    uint64_t* ddr_base_dest = ddr_base_src + 32;
    // Clear the location
    printf("Clearing the destination memory...\n");
    for (int i = 0; i < 16; i++) {
        ddr_base_dest[i] = 0;
    }
    // Prepare the row inputs, the length will be 16 for the MOV instruction
    printf("Writing rows into memory...\n");
    ddr_base_src[0]  = 52  + (55<< 16) + (61ull<<32) + (66ull<< 48);
    ddr_base_src[1]  = 71  + (61<< 16) + (64ull<<32) + (73ull<< 48);
    ddr_base_src[2]  = 63  + (59<< 16) + (66ull<<32) + (90ull<< 48);
    ddr_base_src[3]  = 109 + (585<<16) + (69ull<<32) + (72ull<< 48);
    ddr_base_src[4]  = 62  + (59<< 16) + (68ull<<32) + (113ull<<48);
    ddr_base_src[5]  = 144 + (104<<16) + (66ull<<32) + (73ull<< 48);
    ddr_base_src[6]  = 63  + (58<< 16) + (71ull<<32) + (122ull<<48);
    ddr_base_src[7]  = 154 + (106<<16) + (70ull<<32) + (69ull<< 48);
    ddr_base_src[8]  = 67  + (61<< 16) + (68ull<<32) + (104ull<<48);
    ddr_base_src[9]  = 126 + (88<< 16) + (68ull<<32) + (70ull<< 48);
    ddr_base_src[10] = 79  + (65<< 16) + (60ull<<32) + (70ull<< 48);
    ddr_base_src[11] = 77  + (68<< 16) + (58ull<<32) + (75ull<< 48);
    ddr_base_src[12] = 85  + (71<< 16) + (64ull<<32) + (59ull<< 48);
    ddr_base_src[13] = 55  + (61<< 16) + (65ull<<32) + (83ull<< 48);
    ddr_base_src[14] = 87  + (79<< 16) + (69ull<<32) + (58ull<< 48);
    ddr_base_src[15] = 65  + (76<< 16) + (78ull<<32) + (94ull<< 48);
    // Sanity check input
    printf("Input sanity check\n");
    volatile uint16_t* base_src_in_word = (uint16_t*)ddr_base_src;
    for (int i = 0; i < 64; i++) {
        printf("%d, ", base_src_in_word[i]);
        if (i % 8 == 7) printf("\n");
    }
    printf("Rows in memory.\n");
    // Place the MOV instruction
    printf("Placing MOV instruction in FIFO.\n");
    videox_exec(FUNC_MOV, ddr_base_src, ddr_base_dest, 64);
    videox_exec(FUNC_MOV, ddr_base_src + 8, ddr_base_dest + 8, 64);
    videox_exec(FUNC_MOV, ddr_base_dest, ddr_base_src, 64*127);
    // Wait
    videox_wait();
    // Read from write location and print to UART
    volatile uint16_t* base_dest_in_word = (uint16_t*)ddr_base_dest;
    for (int i = 0; i < 64; i++) {
        printf("%d, ", base_dest_in_word[i]);
        if (i % 8 == 7) printf("\n");
    }
}
