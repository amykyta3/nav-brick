#include <stdint.h>
#include <stdio.h>

void hexdump(void *data, uint16_t size){
    uint8_t *u8_data = data;

    // Print header
    printf("      ");
    for(uint8_t i=0; i<8; i++) printf(" +%x", i);

    for(uint16_t i=0; i<size; i++) {
        if((i & 0x7) == 0){
            printf("\n%04x |", (i / 8) * 8);
        }
        printf(" %02x", u8_data[i]);
    }
    putchar('\n');
}
