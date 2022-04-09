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
  isub SP, 15
  mov R0, [BP-8]
  mov R0, 0
  mov [BP-8], R0
  mov R0, [BP-8]
  iadd R0, 1
  mov [BP-7], R0
  mov R0, [BP-14]
  mov R0, 12
  mov [BP-14], R0
  mov R0, 25
  mov [BP-9], R0
  mov R0, 1
  mov [BP-15], R0
  mov R0, 11
  mov R1, BP
  iadd R1, -11
  mov R2, [BP-15]
  iadd R1, R2
  mov [R1], R0
  mov R0, 22
  mov R1, BP
  iadd R1, -14
  mov R2, [BP-15]
  imul R2, 3
  iadd R1, R2
  iadd R1, 1
  mov [R1], R0
  mov R0, BP
  iadd R0, -14
  mov R1, [BP-15]
  imul R1, 3
  iadd R0, R1
  iadd R0, 1
  mov R0, [R0]
  mov [BP-14], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

