  ; ClearScreen( 0, 0, 128 );
  OUT GPU_ClearColor, 0x000070FF
  OUT GPU_Command, GPUCommand_ClearScreen
  
  ; wait for 1 second
  MOV R0, 60
  
_waiting:
  WAIT
  ISUB R0, 1
  IGT R0, 0
  JT R0, _waiting
  
  ; SetActivePage( 0 );
  OUT GPU_SelectedTexture, 0
  
  ; configure a region;
  OUT GPU_SelectedRegion, 1
  OUT GPU_RegionMinX, 294
  OUT GPU_RegionMinY, 32
  OUT GPU_RegionMaxX, 333
  OUT GPU_RegionMaxY, 51
  OUT GPU_RegionHotSpotX, 314
  OUT GPU_RegionHotSpotY, 47
  
  ; DrawActiveRegion( 320, 180 );
  OUT GPU_DrawingPointX, 320
  OUT GPU_DrawingPointY, 180
  OUT GPU_Command, GPUCommand_DrawRegion
  
  ; return;
  HLT
