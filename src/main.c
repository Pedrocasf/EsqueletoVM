#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "RV32I/RV32I.h"

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 256;

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("No File to execute specified");
        return -1;
    }
    //printf("Executable path: %s\n",argv[argc-1]);
    struct VM_state* RV32I_State = NULL;
    build_vm_state(&RV32I_State, argv[argc-1]);
    //printf("returned from state init\n");
    if(RV32I_State == NULL){
        printf("Couldn't initialize vm state");
        return -2;
    }
    begin(RV32I_State);
    return 0;
} 