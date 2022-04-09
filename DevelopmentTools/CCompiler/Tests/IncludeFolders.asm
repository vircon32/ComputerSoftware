; location of global variables
  define global_number 1
  define global_number2 2

; program start section
  call __global_scope_initialization
  call __function_main
  halt

__global_scope_initialization:
  push BP
  mov BP, SP
  mov R0, 7
  mov [global_number], R0
  mov R0, 15
  mov [global_number2], R0
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  mov R0, [global_number]
  iadd R0, 3
  mov [global_number], R0
  mov R0, [global_number2]
  iadd R0, 5
  mov [global_number2], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

