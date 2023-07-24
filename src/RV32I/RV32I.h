//
// Created by pedrostarling2000 on 7/23/23.
//

#ifndef ESQUELETOVM_RV32I_H
#define ESQUELETOVM_RV32I_H
#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "async.h"
typedef struct VM_state{
    uint32_t pc;
    uint32_t* x;
     void* memory;
    uint32_t* csr;
}VM_state;
void build_vm_state(VM_state** state, char* rom_name);
typedef uint32_t (*VM_Call)(uint32_t pc, void* memory);
typedef VM_Call* (*VM_Instruction)(uint32_t pc, uint32_t *x, void* memory);
void begin(VM_state* state);
#endif //ESQUELETOVM_RV32I_H
