; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables
  define global_VG 0
  define global_StringG 2

__global_scope_initialization:
  push BP
  mov BP, SP
  mov R0, [1]
  mov [global_VG], R0
  mov R0, [2]
  mov [1], R0
  mov DR, global_StringG
  mov SR, __literal_string_22
  mov CR, 7
  movs
  mov SP, BP
  pop BP
  ret

__function_rand:
  push BP
  mov BP, SP
  mov R0, 7
__function_rand_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 36
  mov R0, [14]
  mov [BP-1], R0
  call __function_rand
  mov R1, R0
  iadd R1, 7
  mov R0, R1
  mov [BP-2], R0
  mov R0, [1]
  mov [BP-4], R0
  mov R0, [2]
  mov [BP-3], R0
  mov R0, [3]
  mov [BP-13], R0
  mov R0, [4]
  mov [BP-12], R0
  mov R0, [20]
  mov [BP-11], R0
  mov R0, [15]
  mov [BP-10], R0
  lea DR, [BP-9]
  mov SR, __literal_string_48
  mov CR, 5
  movs
  lea R12, [BP-4]
  lea DR, [BP-15]
  mov CR, 2
  movs
  lea R12, [BP-15]
  lea DR, [BP-24]
  mov CR, 2
  movs
  mov R0, [40]
  mov [BP-22], R0
  mov R0, [30]
  mov [BP-21], R0
  lea DR, [BP-20]
  mov SR, __literal_string_58
  mov CR, 3
  movs
  mov R0, [1]
  mov [BP-28], R0
  mov R0, [2]
  mov [BP-27], R0
  mov R0, [3]
  mov [BP-26], R0
  mov R0, [4]
  mov [BP-25], R0
  call __function_rand
  mov [BP-30], R0
  call __function_rand
  mov [BP-29], R0
  mov R0, [4]
  mov [BP-36], R0
  mov R0, [5]
  mov [BP-35], R0
  mov R0, [6]
  mov [BP-34], R0
  mov R0, [7]
  mov [BP-33], R0
  mov R0, [8]
  mov [BP-32], R0
  mov R0, [9]
  mov [BP-31], R0
__function_main_return:
  mov SP, BP
  pop BP
  ret

__literal_string_22:
  string "Global"
__literal_string_48:
  string "Hola"
__literal_string_58:
  string "Hi"
