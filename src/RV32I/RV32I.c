//
// Created by pedrostarling2000 on 7/23/23.
//

#include "RV32I.h"
const uint32_t KIB_SIZE = 1024;
const uint32_t MiB_SIZE = 1024 * KIB_SIZE;
const uint32_t GiB_SIZE = 1024 * MiB_SIZE;
const uint32_t MAX_ROM_SIZE = 64 * MiB_SIZE;
const uint32_t CSR_COUNT = 4 * KIB_SIZE;
const uint32_t VIO_ADDR = 256 * MiB_SIZE;
const uint32_t X_COUNT = 32;
static uint32_t size= 0;
static uintptr_t io_addr = 0;
static uintptr_t read_slot = 0;
static uint32_t write_slot = 0;
typedef enum{
    R_BYTE,
    R_HALF,
    R_WORD,
    R_DWORD,
    W_BYTE,
    W_HALF,
    W_WORD,
    W_DWORD,
    NONE
} ReadWriteKind;
static ReadWriteKind RWK_slot = NONE;
void catch_sigsegv(int sig, siginfo_t *info, void *ucontext);
void build_vm_state(VM_state** state, char* rom_name){
    uint32_t page_size = getpagesize();
    //printf("page size is %d bytes\n",page_size);
    *state = (VM_state*)calloc(1,sizeof(VM_state));
    //printf("alloc vm state struct\n");
    (*state)->x = calloc(X_COUNT, sizeof(uint32_t));
    (*state)->csr = mmap(NULL,CSR_COUNT * sizeof(uint32_t),PROT_READ|PROT_EXEC|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    FILE *rom_ptr = NULL;
    rom_ptr = fopen(rom_name, "rb");
    if (rom_ptr == NULL){
        printf("Could not open file '%s'\n",
                  rom_name);
    }else{
        //printf("opened ROM\n");
        fseek(rom_ptr,0L,SEEK_END);
        size = ftell(rom_ptr);
        size = (size/page_size)+1;
        size = size*page_size;
        fseek(rom_ptr, 0L, SEEK_SET);
        if(size <= MAX_ROM_SIZE){
            fclose(rom_ptr);
            int rom_fd = open(rom_name,O_RDONLY);
            (*state)->memory = mmap(NULL, 4+GiB_SIZE,PROT_READ|PROT_EXEC|PROT_WRITE,MAP_PRIVATE,rom_fd,0);
            //printf("ALLOCATED Memeory\n");
            mprotect((*state)->memory+VIO_ADDR,8,PROT_NONE);
            //printf("mprotect mmio\n");
            io_addr = (uintptr_t)(*state)->memory+0x10000000; //- (uintptr_t)(void*)(*state)->memory;
            //printf("set allocated mmio global addr\n");
            close(rom_fd);
            //printf("returning from state init\n");
            return;
        }else{
            printf("ROM file too big\n");
        }
    }
}
static inline __attribute__((always_inline)) uint8_t rearrange(uint32_t instruction){
    return ((instruction & (0x00000007<<12))>>7) | ((instruction>>2)&0x1F);
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
static inline __attribute__((always_inline)) uint32_t u_imm(uint32_t instruction){
    return instruction & 0xFFFFF000;
}
static inline __attribute__((always_inline)) uint32_t i_imm(uint32_t instruction){
    return (((int32_t)instruction)>>20);
}
static inline __attribute__((always_inline)) uint8_t shamt(uint32_t instruction){
    return i_imm(instruction) & 0x1F;
}
static inline __attribute__((always_inline)) int32_t b_imm(uint32_t instruction) {
    return (int32_t)((int32_t)(instruction & 0x80000000) >> 19)
    | ((instruction & 0x80) << 4)
      | ((instruction >> 20) & 0x7e0)
      | ((instruction >> 7) & 0x1e);
}
static inline __attribute__((always_inline))  uint32_t s_imm(uint32_t instruction) {
    return ((int32_t)(instruction & 0xfe000000) >> 20)
           | ((instruction >> 7) & 0x1f);
}
static inline __attribute__((always_inline)) int32_t j_imm(uint32_t instruction) {
    return (int32_t) (((int32_t) (instruction & 0x80000000) >> 11)
           | (instruction & 0xff000)
           | ((instruction >> 9) & 0x800)
           | ((instruction >> 20) & 0x7fe));
}
static inline __attribute__((always_inline)) bool funct7(uint32_t instruction){
    return (instruction & 0x40000000) >> 31;
}
static inline __attribute__((always_inline)) void fetch_decode(uint32_t* pc, uint32_t*x, uint8_t*mem, uint32_t* csr);
void HALT(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    printf("full instr %032bb %08hhX\n", *pc, * pc);
    uint8_t instr = rearrange(*pc);
    printf("rearranged instr %08bb %d \n", instr, instr);
    printf("HALTED at addr  %08llX\n",((uint8_t*)pc-mem));
    munmap(mem,4*GiB_SIZE);
    free(x);
    exit(-3);
};
void AUIPC (uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("AUIPC\n");
    uint32_t instr =  *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = u_imm(instr)+ ((uint8_t*)pc-mem);;
    return fetch_decode(++pc,x,mem,csr);
}
void LUI (uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("LUI\n");
    uint32_t instr =  *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = u_imm(instr);
    return fetch_decode(++pc,x,mem,csr);
}
void ADDI(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("ADDI\n");
    uint32_t instr = *pc;
    uint8_t reg_dest = rd(instr);
    uint8_t reg_src = rs1(instr);
    *(x+reg_dest) = i_imm(instr) + *(x+reg_src);
    return fetch_decode(++pc,x,mem,csr);
}

void BEQ (uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("BEQ\n");
    uint32_t instr = *pc;
    uint8_t reg_src1 = rs1(instr);
    uint8_t reg_src2 = rs2(instr);
    if(*(x+reg_src1) == *(x+reg_src2)){
        pc+= b_imm(instr)>>2;
        return fetch_decode(pc,x,mem,csr);
    }else{
        return fetch_decode(++pc,x,mem,csr);
    }
}
void LB(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("LB\n");
    uint32_t instr = *pc;
    uint8_t reg_src1 = rs1(instr);
    uint8_t reg_dest = rd(instr);
    RWK_slot = R_BYTE;
    read_slot = (uintptr_t)x+reg_dest;
    *(x+reg_dest) = *(mem+(*(x+reg_src1) + i_imm(instr)));
    return fetch_decode(++pc,x,mem,csr);
}
void SB(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //struct sigaction act = {0};
    //act.sa_sigaction = catch_sigsegv;
    //act.sa_flags = SA_SIGINFO;
    //sigaction(SIGSEGV, &act, &act);
    //printf("SB\n");
    uint32_t instr = *pc;
    uint8_t reg_src1 = rs1(instr);
    uint8_t reg_src2 = rs2(instr);
    RWK_slot = W_BYTE;
    write_slot = (uint8_t)*(x+reg_src2);
    *(mem+*(x+reg_src1)+s_imm(instr)) = (uint8_t)*(x+reg_src2);
    return fetch_decode(++pc,x,mem,csr);
}
void SW(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //struct sigaction act = {0};
    //act.sa_sigaction = catch_sigsegv;
    //act.sa_flags = SA_SIGINFO;
    //sigaction(SIGSEGV, &act, &act);
    //printf("SW\n");
    uint32_t instr = *pc;
    uint8_t reg_src1 = rs1(instr);
    uint8_t reg_src2 = rs2(instr);
    RWK_slot = W_WORD;
    write_slot = *(x+reg_src2);
    *( uint32_t *)(mem+*(x+reg_src1)+s_imm(instr)) = *(x+reg_src2);
    return fetch_decode(++pc,x,mem,csr);
}
void BNE (uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("BNE\n");
    uint32_t instr = *pc;
    uint8_t reg_src1 = rs1(instr);
    uint8_t reg_src2 = rs2(instr);
    if(*(x+reg_src1) != *(x+reg_src2)){
        pc+= b_imm(instr)>>2;
        return fetch_decode(pc,x,mem,csr);
    }else{
        return fetch_decode(++pc,x,mem,csr);
    }
}
void JAL(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr){
    //printf("JAL\n");
    uint32_t instr = *pc;
    uint8_t reg_dest = rd(instr);
    *(x+reg_dest) = (uint32_t)(pc - (uint32_t*)mem);
    pc+= j_imm(instr)>>2;
    return fetch_decode(pc,x,mem,csr);

}
void catch_sigsegv(int sig, siginfo_t *info, void *ucontext) {
    ucontext_t *ctx = (ucontext_t *)ucontext;
    //printf ("\n Signal %d received",sig);
    uintptr_t addr = (uintptr_t)(void*)info->si_addr;
    //printf ("\n at address %lx",addr);
    uintptr_t mmio_addr = addr - io_addr;
    //printf ("\n at IO address %lx",mmio_addr);
    switch (mmio_addr) {
        case 0:
            if (RWK_slot == W_BYTE) {
                printf("%c", (uint8_t) write_slot);
            }
            break;
        case 4:
            if (RWK_slot == W_WORD) {
                if (write_slot == 0x00005555) {
                    exit(0);
                }
            }
            break;
    }
#if __x86_64
    (ctx->uc_mcontext.gregs[16]) += 4;
#elif __aarch64__
    (ctx->uc_mcontext.pc) += 4;
#elif __riscv
    (ctx->uc_mcontext.__gregs[1]) += 4;
#else
#error "only x86_64, arm64 and RISC-V64 supported"
#endif
}

void (*decode_table[256])(uint32_t* pc, uint32_t* x, uint8_t* mem, uint32_t* csr) = {
   LB,
   HALT,
   HALT,
   HALT,
   ADDI,
   AUIPC,
   HALT,
   HALT,
   SB,
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
   BEQ,
   HALT,
   HALT,
   JAL,
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
   BNE,
   HALT,
   HALT,
   JAL,
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
   SW,
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
   JAL,
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
   JAL,
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
   JAL,
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
   JAL,
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
   JAL,
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
   JAL,
   HALT,
   HALT,
   HALT,
   HALT
};
static inline __attribute__((always_inline)) void fetch_decode(uint32_t* pc, uint32_t*x, uint8_t*mem, uint32_t*csr ){
    //printf("PC:%08llX INSTR:%08llX RE:%02llX\n", ((uint8_t*)pc-mem), *pc, rearrange(*pc));
    *x = 0;
    return (decode_table[rearrange(*pc)])(pc,x,mem,csr);
}
void begin(VM_state* state){
    //printf("begin vm execution");
    struct sigaction act = {0};
    act.sa_sigaction = catch_sigsegv;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, &act);
    state->pc = ( uint32_t *)state->memory;
    //printf("full instr %032bb  %08hhX\n", *state->pc, *state->pc);
    //uint8_t actual_instr = rearrange(*state->pc);
    //printf("rearranged instr %08bb\n", actual_instr);
    fetch_decode(state->pc, state->x, state->memory,state->csr);
}