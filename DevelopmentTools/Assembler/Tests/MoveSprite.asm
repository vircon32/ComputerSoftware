%define SpriteX 0x1
%define SpriteY 0x2
  
jmp _main
; ------------------------
  
_draw_scene:

  ; clear the screen
  out GPU_ClearColor, 0x000070FF
  out GPU_Command, GPUCommand_ClearScreen
  
  ; draw the sprite at current position
  mov R0, [SpriteX]
  mov R1, [SpriteY]
  out GPU_DrawingPointX, R0
  out GPU_DrawingPointY, R1
  out GPU_Command, GPUCommand_DrawRegion
  ret
  
_main:

  ; create a region
  out GPU_SelectedTexture, 0
  out GPU_SelectedRegion, 1
  out GPU_RegionMinX, 294
  out GPU_RegionMinY, 32
  out GPU_RegionMaxX, 333
  out GPU_RegionMaxY, 51
  out GPU_RegionHotSpotX, 314
  out GPU_RegionHotSpotY, 47
  
  ; set initial position
  mov R0, 320
  mov R1, 180
  mov [SpriteX], R0
  mov [SpriteY], R1
  
_check_left:

  in R0, INP_GamepadLeft
  ilt R0, 0
  jt R0, _check_right
  mov R1, [SpriteX]
  iadd R1, -1
  mov [SpriteX], R1
  
_check_right:

  in R0, INP_GamepadRight
  ilt R0, 0
  jt R0, _check_up
  mov R1, [SpriteX]
  iadd R1, 1
  mov [SpriteX], R1
  
_check_up:

  in R0, INP_GamepadUp
  ilt R0, 0
  jt R0, _check_down
  mov R1, [SpriteY]
  iadd R1, -1
  mov [SpriteY], R1
  
_check_down:

  in R0, INP_GamepadDown
  ilt R0, 0
  jt R0, _check_button
  mov R1, [SpriteY]
  iadd R1, 1
  mov [SpriteY], R1

_check_button:

  in R0, INP_GamepadButtonA
  ilt R0, 0
  jt R0, _end_frame
  hlt

_end_frame:

  call _draw_scene
  wait
  jmp _check_left