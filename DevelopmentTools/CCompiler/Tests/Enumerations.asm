; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 5
  mov R0, 1
  mov [BP-1], R0
  mov R0, 0
  mov [BP-2], R0
  mov R0, [BP-1]
  mov [BP-3], R0
  mov R0, 2
  mov [BP-4], R0
  mov R0, [BP-1]
  mov R1, [BP-2]
  ilt R0, R1
  mov R0, [BP-1]
  ieq R0, 0
  mov R0, [BP-1]
  ine R0, 0
  mov R0, [BP-1]
  ilt R0, 0
  mov R0, [BP-1]
  ige R0, 3
  mov R0, 2
  mov [BP-5], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

