; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables
  define global_AimTrajectory 0

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 1
  mov R0, global_AimTrajectory
  mov [BP-1], R0
  mov R0, [BP-1]
  mov R1, R0
  iadd R1, 5
  mov [BP-1], R1
  mov R0, [BP-1]
  mov R1, R0
  isub R1, 5
  mov [BP-1], R1
  mov R0, [BP-1]
  iadd R0, 5
  mov [BP-1], R0
  mov R0, [BP-1]
  isub R0, 5
  mov [BP-1], R0
  mov R0, [BP-1]
  iadd R0, 50
  mov R0, [BP-1]
  isub R0, 50
__function_main_return:
  mov SP, BP
  pop BP
  ret

