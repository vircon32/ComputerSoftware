; location of global variables

; program start section
  call __global_scope_initialization
  call __function_main
  halt

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_DoNothing:
  push BP
  mov BP, SP
__function_DoNothing_return:
  mov SP, BP
  pop BP
  ret

__function_sum_floats:
  push BP
  mov BP, SP
  isub SP, 1
  mov R0, [BP+2]
  mov R1, [BP+3]
  fadd R0, R1
  cfi R0
  mov [BP-1], R0
  mov R0, [BP-1]
  cif R0
__function_sum_floats_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 3
  mov R0, 7
  mov [BP-1], R0
  mov R1, 3
  cif R0
  mov [SP], R1
  mov R1, [BP-1]
  cif R0
  mov [SP+1], R1
  call __function_sum_floats
__function_main_return:
  mov SP, BP
  pop BP
  ret

