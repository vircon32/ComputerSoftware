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
  mov R0, 25
  mov R1, __embedded_matrix
  iadd R1, 3
  iadd R1, 2
  mov [R1], R0
  mov R0, __embedded_matrix
  iadd R0, 3
  iadd R0, 2
  mov R0, [R0]
  mov R1, __embedded_matrix
  mov [R1], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

__embedded_matrix:
  datafile "EmbeddedMatrix.bin"
