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
  
  ;ptrs[0];
  mov R0, [BP-9]
  
  ;ptrs[ 0 ] = NULL;
  mov R0, 0
  mov [BP-9], R0    ;BP-9 = BP-2 - 8 + 1
  
  ;ptrs[ 1 ] = ptrs[ 0 ] + 1;
  mov R0, [BP-9]
  iadd R0, 1
  mov [BP-8], R0
  
  ;matrix[0][0];
  mov R0, [BP-15]   ;BP-15 = BP-9 - 6
  
  ;matrix[0][0] = 12;
  mov R0, 12
  mov [BP-15], R0
  
  ;matrix[1][2] = 25;
  mov R0, 25
  mov [BP-10], R0   ;BP-10 = BP-15 + 1*3 + 2*1
  
  ;int index = 1;
  mov R0, 1
  mov [BP-16], R0   ;BP-16 = BP-15 - 1
  
  ;matrix[1][index] = 11;
  mov R0, 11        ;R0 = value
  mov R1, BP        ;R1 = BP-12 = (BP-15)+3 = &matrix[1]
  iadd R1, -12
  mov R2, [BP-16]   ;R2 = index
  iadd R1, R2       ;R1 = (BP-12)+index = &matrix[1][index]
  mov [R1], R0      ;copy value to matrix[1][index]
  
  ;matrix[index][1] = 22;
  mov R0, 22        ;R0 = value
  mov R1, BP        ;R1 = BP-15 = &matrix
  iadd R1, -15
  mov R2, [BP-16]   ;R2 = index
  imul R2, 3        ;R2 = index*3 = offset of matrix[index]
  iadd R1, R2       ;R1 = &matrix + offset = &matrix[index]
  iadd R1, 1        ;R1 = &matrix[index] + 1 = &matrix[index][1]
  mov [R1], R0      ;copy value to matrix[index][1]
  
  ;matrix[0][0] = matrix[index][1];
  mov R0, BP        ;R0 = BP-15 = &matrix
  iadd R0, -15      
  mov R1, [BP-16]   ;R1 = index
  imul R1, 3        ;R1 = index*3 = offset of matrix[index]
  iadd R0, R1       ;R0 = &matrix + offset = &matrix[index]
  iadd R0, 1        ;R0 = &matrix[index][1]
  mov R0, [R0]      ;R0 = matrix[index][1]
  mov [BP-15], R0   ;copy value to matrix[0][0]
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

