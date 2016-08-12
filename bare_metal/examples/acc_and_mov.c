#include <stdio.h>
#include "uart.h"
#include "memory.h"
#include "encoding.h"

volatile uint32_t *mem_base;

volatile uint32_t * get_videomem_base() {
    return (volatile uint32_t *)(DEV_MAP__io_ext_video_acc_inst__BASE);
}

static void video_add_inst(uint32_t val) {
    volatile uint32_t *reg = &((volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE)[0];
    *reg = val;
}

static uint32_t video_peak_inst() {
    volatile uint32_t *reg = &((volatile uint32_t *)DEV_MAP__io_ext_video_acc_inst__BASE)[0];
    return *reg;
}

int main() {
  uart_init();
  uint64_t* ddr_base_src  = (uint64_t*)(void*)get_ddr_base();
  uint64_t* ddr_base_dest = ddr_base_src + 32;
  // Clear the location
  printf("Clearing the destination memory...\n");
  for (int i = 0; i < 16; i++) {
    ddr_base_dest[i] = 0;
  }
  // Prepare for read
  printf("Preparing to load read base address...\n");
  uint32_t full_load_rd = 2;
  uint32_t rd_lower = (uint32_t)((uintptr_t)ddr_base_src);
  uint32_t rd_upper = (uint32_t)((uintptr_t)ddr_base_src >> 32);
  printf("Sending instruction\n");
  video_add_inst(full_load_rd);
  video_add_inst(rd_lower);
  video_add_inst(rd_upper);
  printf("Read base address instructions sent.\n");
  // Prepare write location
  printf("Preparing to load write base address...\n");
  uint32_t full_load_wr = (1<<27) + 2;
  uint32_t wr_lower = (uint32_t)((uintptr_t)ddr_base_dest);
  uint32_t wr_upper = (uint32_t)((uintptr_t)ddr_base_dest >> 32);;
  video_add_inst(full_load_wr);
  video_add_inst(wr_lower);
  video_add_inst(wr_upper);
  printf("Write base address instructions sent.\n");
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
  video_add_inst(1+(1<<20)+(1<<13)+(0<<6));
  // Wait
  while(video_peak_inst() != 0){
    printf("Waiting on MOV!\n");
  }
  // Read from write location and print to UART
  volatile uint16_t* base_dest_in_word = (uint16_t*)ddr_base_dest;
  for (int i = 0; i < 64; i++) {
    printf("%d, ", base_dest_in_word[i]);
    if (i % 8 == 7) printf("\n");
  }
}
