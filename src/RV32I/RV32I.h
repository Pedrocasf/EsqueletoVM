//
// Created by pedrostarling2000 on 7/23/23.
//

#ifndef ESQUELETOVM_RV32I_H
#define ESQUELETOVM_RV32I_H
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <errno.h>
#include <fcntl.h>
#include <linux/mman.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <unistd.h>

typedef struct VM_state{
    uint32_t* pc;
    uint32_t* x;
    uint8_t* memory;
    uint32_t* csr;
}VM_state;

void build_vm_state(VM_state** state, char* rom_name);

void begin(VM_state* state);
#endif //ESQUELETOVM_RV32I_H
