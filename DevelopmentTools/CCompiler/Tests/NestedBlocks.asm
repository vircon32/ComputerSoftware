; location of global variables
  define global_Global1 1
  define global_Global2 2

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
  isub SP, 5
  mov R0, [BP-4]
  mov R1, [BP-5]
  iadd R0, R1
  mov [BP-6], R0
  mov R0, 17
  mov [BP-4], R0
  mov R0, [BP-2]
  mov [BP-3], R0
  mov R0, [global_Global1]
  iadd R0, 1
  mov [global_Global1], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

