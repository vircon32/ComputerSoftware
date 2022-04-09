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
  
  ; 1* Word + 1* int
  isub SP, 4
  mov R0, 10
  mov [BP-1], R0
  mov R0, [BP-1]
  isgn R0
  cif R0
  mov [BP-1], R0
  mov R0, [BP-3]
  mov [BP-2], R0
  
  ; int s = sizeof( MultiWord );
  mov R0, 4
  mov [BP-4], R0
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

