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

__function_Two:
  push BP
  mov BP, SP
  mov R0, 2
__function_Two_return:
  mov SP, BP
  pop BP
  ret

__function_Four:
  push BP
  mov BP, SP
  push R1
  push R2
  call __function_Two
  mov R1, R0
  call __function_Two
  mov R2, R0
  iadd R1, R2
  mov R0, R1
__function_Four_return:
  pop R2
  pop R1
  mov SP, BP
  pop BP
  ret

__function_Double:
  push BP
  mov BP, SP
  mov R0, [BP+2]
  imul R0, 2
__function_Double_return:
  mov SP, BP
  pop BP
  ret

__function_Twelve:
  push BP
  mov BP, SP
  isub SP, 2
  push R1
  push R2
  isub SP, 1
  call __function_Two
  mov [BP-2], R0
  mov R2, [BP-2]
  mov [SP], R2
  call __function_Double
  mov R1, R0
  mov [BP-1], R1
  mov R0, R1
  mov R0, [BP-1]
  imul R0, 3
__function_Twelve_return:
  iadd SP, 1
  pop R2
  pop R1
  mov SP, BP
  pop BP
  ret

__function_Empty:
  push BP
  mov BP, SP
__function_Empty_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 1
  call __function_Two
  mov R1, R0
  call __function_Four
  mov R2, R0
  imul R2, 3
  iadd R1, R2
  mov [BP-1], R1
  mov R0, R1
__function_main_return:
  mov SP, BP
  pop BP
  ret

