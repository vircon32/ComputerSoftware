; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables
  %define global_s1 0
  %define global_s2 1
  %define global_s3 2

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_add:
  push BP
  mov BP, SP
  push R1
  mov R0, [BP+2]
  mov R1, [BP+3]
  iadd R0, R1
__function_add_return:
  pop R1
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 3
  
  ; int( int, int )* fptr = &add;
  mov R0, __function_add
  mov [BP-1], R0
  
  ; s1 = add( 10, 20 );
  mov R2, 10
  mov [SP], R2
  mov R2, 20
  mov [SP+1], R2
  call __function_add
  mov R1, R0
  mov [global_s1], R1
  mov R0, R1
  
  ; s2 = fptr( 30, 40 );
  mov R2, 30
  mov [SP], R2
  mov R2, 40
  mov [SP+1], R2
  mov R3, [BP-1]   ; R3 = fptr
  call R3          ; call fprt
  mov R1, R0
  mov [global_s2], R1
  mov R0, R1
  
  ; s3 = (*fptr)( 50, 60 );
  mov R2, 50
  mov [SP], R2
  mov R2, 60
  mov [SP+1], R2
  mov R3, [BP-1]   ; R3 = fptr
  call R3          ; call fptr
  mov R1, R0
  mov [global_s3], R1
  mov R0, R1
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

