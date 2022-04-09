; location of global variables
  define global_ptrs 1      ;avoids 0 for NULL
  define global_matrix 9    ;=1+8
  define global_index 15    ;=9+2*3

; program start section
  call __global_scope_initialization
  call __function_main
  halt

__global_scope_initialization:
  push BP
  mov BP, SP
  
  ;int index = 1;
  mov R0, 1
  mov [global_index], R0
  
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  
  ;ptrs[0];
  mov R0, [global_ptrs]
  
  ;ptrs[ 0 ] = NULL;
  mov R0, 0
  mov [global_ptrs], R0
  
  ;ptrs[ 1 ] = ptrs[ 0 ] + 1;
  mov R0, [global_ptrs]
  iadd R0, 1
  mov [2], R0
  
  ;matrix[0][0];
  mov R0, [global_matrix]
  
  ;matrix[0][0] = 12;
  mov R0, 12
  mov [global_matrix], R0
  
  ;matrix[1][2] = 25;
  mov R0, 25
  mov [14], R0
  
  ;matrix[1][index] = 11;
  mov R0, 11
  mov R1, 12
  mov R2, [global_index]
  iadd R1, R2
  mov [R1], R0
  
  ;matrix[index][1] = 22;
  mov R0, 22
  mov R1, global_matrix
  mov R2, [global_index]
  imul R2, 3
  iadd R1, R2
  iadd R1, 1
  mov [R1], R0
  
  ;matrix[0][0] = matrix[index][1];
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

