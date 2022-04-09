__hardware_error_vector:
  jmp __program_start_section
  call __function_error_handler
  hlt

; program start section
__program_start_section:
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables
  define global_ptrs 1
  define global_matrix 9
  define global_index 15

__global_scope_initialization:
  push BP
  mov BP, SP
  mov R0, 1
  mov [global_index], R0
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  mov R0, [global_ptrs]
  mov R0, 0
  mov [global_ptrs], R0
  mov R0, [global_ptrs]
  iadd R0, 1
  mov [2], R0
  mov R0, [global_matrix]
  mov R0, 12
  mov [global_matrix], R0
  mov R0, 25
  mov [14], R0
  mov R0, 11
  mov R1, 12
  mov R2, [global_index]
  iadd R1, R2
  mov [R1], R0
  mov R0, 22
  mov R1, global_matrix
  mov R2, [global_index]
  imul R2, 3
  iadd R1, R2
  iadd R1, 1
  mov [R1], R0
  mov R0, global_matrix
  mov R1, [global_index]
  imul R1, 3
  iadd R0, R1
  iadd R0, 1
  mov R0, [R0]
  mov [global_matrix], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

__function_error_handler:
  push BP
  mov BP, SP
__function_error_handler_return:
  mov SP, BP
  pop BP
  ret

