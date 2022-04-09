  define GPUCommandClearScreen 1
  define GPUCommandDrawRegion  2
  
  define SpriteX 0x1
  define SpriteY 0x2
  
  jmp _main
  ; ------------------------
  
_draw_scene:

  ; clear the screen
  out GPUClearColor, 0x000070FF
  out GPUCommand, GPUCommandClearScreen
  
  ; draw the sprite at current position
  mov R0, [SpriteX]
  mov R1, [SpriteY]
  out GPUDrawingPointX, R0
  out GPUDrawingPointY, R1
  out GPUCommand, GPUCommandDrawRegion
  ret
  
_main:

  ; create a region
  out GPUActivePage, 0
  out GPUActiveRegion, 1
  out GPURegionMinX, 294
  out GPURegionMinY, 32
  out GPURegionWidth, 40
  out GPURegionHeight, 20
  out GPURegionHotSpotX, 20
  out GPURegionHotSpotY, 15
  
  ; set initial position
  mov R0, 320
  mov R1, 180
  mov [SpriteX], R0
  mov [SpriteY], R1
  
_check_left:

  in R0, INPGamepad1Left
  ilt R0, 0
  jt R0, _check_right
  mov R1, [SpriteX]
  iadd R1, -1
  mov [SpriteX], R1
  
_check_right:

  in R0, INPGamepad1Right
  ilt R0, 0
  jt R0, _check_up
  mov R1, [SpriteX]
  iadd R1, 1
  mov [SpriteX], R1
  
_check_up:

  in R0, INPGamepad1Up
  ilt R0, 0
  jt R0, _check_down
  mov R1, [SpriteY]
  iadd R1, -1
  mov [SpriteY], R1
  
_check_down:

  in R0, INPGamepad1Down
  ilt R0, 0
  jt R0, _check_button
  mov R1, [SpriteY]
  iadd R1, 1
  mov [SpriteY], R1

_check_button:

  in R0, INPGamepad1ButtonA
  ilt R0, 0
  jt R0, _end_frame
  halt

_end_frame:

  call _draw_scene
  wait
  jmp _check_left