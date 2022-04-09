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
  
  ; 1* Box + 3* Int + 1* Pointer
  ; Box is 4 words: &Box = (BP-1)-3 = BP-4
  isub SP, 8
  
  ; int TopY = B.TopLeft.y;
  mov R0, [BP-3]   ; &B.TopLeft.y = BP-3
  mov [BP-5], R0   ; &TopY = BP-5
  
  ; Box* PointedBox = &B;
  lea R0, [BP-4]
  mov [BP-6], R0   ; &PointedBox = BP-6
  
  ; int LeftX = PointedBox->TopLeft.x;
  mov R1, [BP-6]
  mov R0, [R1]
  mov [BP-7], R0   ; &LeftX = BP-7
  
  ; int RightX = (*PointedBox).BottomRight.x;
  lea R1, [BP-6]
  mov R1, [R1]
  iadd R1, 2
  mov R0, [R1]
  mov [BP-8], R0   ; &RightX = BP-8
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

