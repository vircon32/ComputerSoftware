; location of global variables
  define global_Array 1

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

__function_Function1:
  push BP
  mov BP, SP
__function_Function1_return:
  mov SP, BP
  pop BP
  ret

__function_Function2:
  push BP
  mov BP, SP
__function_Function2_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
__function_main_return:
  mov SP, BP
  pop BP
  ret

