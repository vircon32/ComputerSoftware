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
  isub SP, 4
  
  ;int* ptr = &var1;
  mov R0, BP
  iadd R0, -2
  mov [BP-4], R0
  
  ;++ptr;
  mov R0, [BP-4]
  iadd R0, 1
  mov [BP-4], R0
  
  ;int** ptrptr = &ptr;
  mov R0, BP
  iadd R0, -4
  mov [BP-5], R0
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

