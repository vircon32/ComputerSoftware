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

__function_test:
  push BP
  mov BP, SP
__function_test_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 7
  mov R0, __literal_string_8
  mov [BP-1], R0
  mov R0, BP
  iadd R0, -4
  mov [BP-5], R0
  mov R0, 2
  mov [BP-6], R0
  mov R0, BP
  iadd R0, -4
  mov R1, [BP-6]
  iadd R0, R1
  mov R0, [R0]
  mov [BP-4], R0
  mov R1, BP
  iadd R1, -4
  mov [SP], R1
  call __function_test
  mov R0, [BP-3]
  mov R0, __literal_string_34
  iadd R0, 3
  mov R0, [R0]
  mov R0, BP
  iadd R0, -4
__function_main_return:
  mov SP, BP
  pop BP
  ret

__literal_string_8:
  string "hello"
__literal_string_34:
  string "goodbye"
