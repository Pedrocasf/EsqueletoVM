//
// Created by pedrostarling2000 on 7/23/23.
//

#include "RV32I.h"
const uint32_t KIB_SIZE = 1024;
const uint32_t MiB_SIZE = 1024 * KIB_SIZE;
const uint32_t MAX_ROM_SIZE = 64 * MiB_SIZE;
const uint32_t RAM_SIZE = 8 * MiB_SIZE;
const uint32_t VRAM_SIZE = MiB_SIZE;
const uint32_t CSR_COUNT = 4 * KIB_SIZE;
const uint32_t X_COUNT = 32;
void build_vm_state(VM_state** state, char* rom_name){
    *state = (VM_state*)calloc(1,sizeof(VM_state));
    (*state)->memory = calloc(MAX_ROM_SIZE + RAM_SIZE + VRAM_SIZE, sizeof(uint8_t));
    (*state)->x = calloc(X_COUNT, sizeof(uint32_t));
    (*state)->csr = calloc(CSR_COUNT, sizeof(uint32_t));
    
    FILE *rom_ptr = NULL;
    rom_ptr = fopen(rom_name, "rb");
    if (rom_ptr == NULL){
        printf("Could not open file '%s'\n",
                  rom_name);
    }else{
        fseek(rom_ptr,0L,SEEK_END);
        uint64_t size = ftell(rom_ptr);
        fseek(rom_ptr, 0L, SEEK_SET);
        if(size <= MAX_ROM_SIZE){
            uint32_t result2 = fread((*state)->memory, sizeof(uint8_t), size, rom_ptr);
            if (result2 != size){
                printf("Read file is %d bytes long, but %d bytes were read\n",size, result2);
            }
        }else{
            printf("ROM file too big\n");
        }
        fclose(rom_ptr);
    }
}
static inline __attribute__((always_inline)) uint8_t rearrange(uint32_t instruction){
    return ((instruction & (0x7<<10))>>5) | instruction>>2;
}
void begin(VM_state* state){
    uint32_t* mem = (uint32_t *)state->memory;
    uint32_t full_instr = *(mem+state->pc);
    printf("full instr %032bb\n", full_instr);
    uint8_t actual_instr = rearrange(full_instr);
    printf("rearranged instr %08bb\n", actual_instr);
}
