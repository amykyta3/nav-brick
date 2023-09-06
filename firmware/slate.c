#include <stdio.h>
#include <string.h>
#include "slate.h"
#include "fram.h"

slate_t Slate;



#define FRAM_ENTRY_SPACING  0x20
_Static_assert(sizeof(fram_entry_t) <= FRAM_ENTRY_SPACING, "FRAM_ENTRY_SPACING is too small");

#define MAX_ENTRIES (FRAM_SIZE / FRAM_ENTRY_SPACING)

static uint16_t current_seq_num;

static void load_nv(void);

void slate_init(void){
    current_seq_num = 0;
    load_nv();
}


static uint32_t crc32(void *data, uint8_t size) {
    uint8_t *u8_data = data;
    uint32_t crc;

    crc = 0xFFFFFFFF;
    for(uint8_t i=0; i<size; i++) {
        crc ^= u8_data[i];
        for (uint8_t j=0; j<8; j++) {
            if(crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

void slate_write_nv(fram_entry_t *entry, uint16_t addr){
    entry->crc = crc32(&entry->payload, sizeof(entry_payload_t));
    fram_write(addr, entry, sizeof(fram_entry_t));
}

bool slate_read_nv(fram_entry_t *entry, uint16_t addr){
    fram_read(addr, entry, sizeof(fram_entry_t));

    // Chec if version OK
    if(entry->payload.version != NV_SLATE_VERSION) return false;

    // Check CRC
    uint32_t crc;
    crc = crc32(&entry->payload, sizeof(entry_payload_t));
    if(crc != entry->crc) return false;

    return true;
}

void slate_save_nv(void){
    fram_entry_t entry;

    // prep entry
    current_seq_num++;
    entry.payload.version = NV_SLATE_VERSION;
    entry.payload.seq_num = current_seq_num;

    // Pull data from slate to be saved
    memcpy(&entry.payload.data, &Slate.nv, sizeof(nonvolatile_slate_t));
    entry.payload.data.prev_session_altitude = Slate.current_altitude;

    // Save to FRAM
    uint16_t addr;
    addr = (current_seq_num * FRAM_ENTRY_SPACING) & (FRAM_SIZE-1);
    slate_write_nv(&entry, addr);
}


static void load_nv(void){
    fram_entry_t entry;

    // Search FRAM for highest sequence number that is good
    for(int i=0; i<MAX_ENTRIES; i++){
        uint16_t addr;
        addr = (i * FRAM_ENTRY_SPACING) & (FRAM_SIZE-1);

        bool result;
        result = slate_read_nv(&entry, addr);
        if(!result) continue;

        if(entry.payload.seq_num > current_seq_num) {
            // Found a newer entry. Save it
            current_seq_num = entry.payload.seq_num;
            memcpy(&Slate.nv, &entry.payload.data, sizeof(nonvolatile_slate_t));
            Slate.prev_session_valid = true;
        }
    }

    if(Slate.prev_session_valid) {
        printf("\n\nLoaded NV slate. sn: %04x\n", current_seq_num);
    } else {
        printf("\n\nNo NV slate found\n");
    }
}
