#include <stdbool.h>
#include <stdio.h>

#include "RV32I/RV32I.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    exit(-1);
  }
  printf("Executable path: %s\n", argv[argc - 1]);
  struct VM_state *RV32I_State = NULL;
  build_vm_state(&RV32I_State, argv[argc - 1]);
  printf("returned from state init\n");
  if (RV32I_State == NULL) {
    printf("Couldn't initialize vm state\n");
    exit(-2);
  }
  begin(RV32I_State);
  return 0;
}