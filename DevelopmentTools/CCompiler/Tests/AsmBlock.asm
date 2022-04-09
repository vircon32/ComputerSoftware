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

__function_GetCycleCounter:
  push BP
  mov BP, SP
  isub SP, 1
  push R0
  in R0, TIM_CycleCounter
  mov [BP-2], R0
  pop R0
  mov R0, [BP-2]
__function_GetCycleCounter_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 1
  call __function_GetCycleCounter
  mov [BP-2], R0
  mov R0, [BP-2]
  iadd R0, 1
  mov [BP-2], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

