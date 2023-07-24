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
            (*state)->memory = calloc(size + RAM_SIZE, sizeof(uint8_t));
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
    return ((instruction & (0x7<<12))>>5) | ((instruction>>2)&0x1F);
}
static inline __attribute__((always_inline)) uint8_t rd(uint32_t instruction){
    return (instruction & 0x00000F80) >> 7;
}
static inline __attribute__((always_inline)) uint8_t rs1(uint32_t instruction){
    return (instruction & 0x000F8000) >> 15;
}
static inline __attribute__((always_inline)) uint8_t rs2(uint32_t instruction){
    return (instruction & 0x001F00000) >> 20;
}
void HALT(uint32_t* pc, uint32_t* x, uint8_t* mem){
    printf("full instr %032bb %08hhX\n", *pc, * pc);
    uint8_t instr = rearrange(*pc);
    printf("rearranged instr %08bb %d \n", instr, instr);
    printf("HALTED at addr  %08llX\n",((uint8_t*)pc-mem));
    exit(-3);
};
void AUIPC (uint32_t* pc, uint32_t* x, uint8_t* mem){
    printf("AUIPC\n");
    uint32_t instr =  *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = (instr & 0xFFFFF000)+ ((uint8_t*)pc-mem);;
    return fetch_decode(++pc,x,mem);
}
void ADDI(uint32_t* pc, uint32_t* x, uint8_t* mem){
    printf("ADDI\n");
    uint32_t instr = *pc;
    uint8_t reg_dest = rd(instr);
    uint8_t reg_src = rs1(instr);
    *(x+reg_dest) = (((int32_t)instr)>>20) + *(x+reg_src);
    return fetch_decode(++pc,x,mem);
}
void LUI (uint32_t* pc, uint32_t* x, uint8_t* mem){
    printf("LUI\n");
    uint32_t instr =  *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = (instr & 0xFFFFF000);
    return fetch_decode(++pc,x,mem);
}
void BEQ (uint32_t* pc, uint32_t* x, uint8_t* mem){
    printf("SLLI\n");
    uint32_t instr = *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = (instr & 0xFFFFF000);
    return fetch_decode(++pc,x,mem);
}
void (*decode_table[256])(uint32_t* pc, uint32_t* x, uint8_t* mem) = {
   HALT,
   HALT,
   HALT,
   HALT,
   ADDI,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,//SLLI,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   AUIPC,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   LUI,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT,
   HALT
};
static inline __attribute__((always_inline)) void fetch_decode(uint32_t* pc, uint32_t*x, uint8_t*mem){
    printf("PC:%08llX INSTR:%08llX RE:%02llX\n", ((uint8_t*)pc-mem), *pc, rearrange(*pc));
    *x = 0;
    (decode_table[rearrange(*pc)])(pc,x,mem);
}
void begin(VM_state* state){;
    state->pc = ( uint32_t *)state->memory;
    //printf("full instr %032bb  %08hhX\n", *state->pc, *state->pc);
    //uint8_t actual_instr = rearrange(*state->pc);
    //printf("rearranged instr %08bb\n", actual_instr);
    fetch_decode(state->pc, state->x, state->memory);
}