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

__function_main:
  push BP
  mov BP, SP
  isub SP, 1
  mov R0, 598
  mov [BP-1], R0
__if_start_15:
  mov R0, [BP-1]
  iadd R0, 598
  igt R0, 100
  jf R0, __if_end_15
  mov R0, [BP-1]
  iadd R0, 1
  mov [BP-1], R0
__if_end_15:
__function_main_return:
  mov SP, BP
  pop BP
  ret

