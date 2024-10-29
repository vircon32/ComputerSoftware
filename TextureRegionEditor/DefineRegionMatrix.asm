; begin include guard
%ifndef DEFINE_REGION_MATRIX
%define DEFINE_REGION_MATRIX

; name the 9 subroutine arguments in the stack
%define RegionMinX [SP+9]
%define RegionMinY [SP+8]
%define RegionMaxX [SP+7]
%define RegionMaxY [SP+6]
%define RegionHotX [SP+5]
%define RegionHotY [SP+4]
%define RegionsInX [SP+3]
%define RegionsInY [SP+2]
%define RegionsGap [SP+1]

; name all working registers
%define RegionID R0
%define CounterX R1
%define OffsetX R2
%define OffsetY R3
%define WorkingMinX R4
%define WorkingMaxX R5
%define WorkingHotX R6
%define Aux R7

_define_region_matrix:
  
  ; capture the first region
  in RegionID, GPU_SelectedRegion
  
  ; calculate offset x = (MaxX - MinX + 1) + Gap
  mov OffsetX, RegionMaxX
  mov Aux, RegionMinX
  isub OffsetX, Aux
  iadd OffsetX, 1
  mov Aux, RegionsGap
  iadd OffsetX, Aux
  
  ; calculate offset y = (MaxY - MinY + 1) + Gap
  mov OffsetY, RegionMaxY
  mov Aux, RegionMinY
  isub OffsetY, Aux
  iadd OffsetY, 1
  mov Aux, RegionsGap
  iadd OffsetY, Aux
  
  ; now we can start the loop on Y
  _define_region_matrix_loop_y:
  
    ; initialize the loop on X
    mov WorkingMinX, RegionMinX
    mov WorkingMaxX, RegionMaxX
    mov WorkingHotX, RegionHotX
    mov CounterX, RegionsInX
    
    _define_region_matrix_loop_x:
    
      ; define current region
      out GPU_SelectedRegion, RegionID
      out GPU_RegionMinX, WorkingMinX
      out GPU_RegionMaxX, WorkingMaxX
      out GPU_RegionHotspotX, WorkingHotX
      mov Aux, RegionMinY
      out GPU_RegionMinY, Aux
      mov Aux, RegionMaxY
      out GPU_RegionMaxY, Aux
      mov Aux, RegionHotY
      out GPU_RegionHotspotY, Aux
    
      ; for next iteration increment region ID and advance to the right
      iadd RegionID, 1
      iadd WorkingMinX, OffsetX
      iadd WorkingMaxX, OffsetX
      iadd WorkingHotX, OffsetX
      
      ; repeat loop on X if we did not define the whole row
      isub CounterX, 1
      jt CounterX, _define_region_matrix_loop_x
      
    ; for next iteration advance down
    mov Aux, RegionMinY
    iadd Aux, OffsetY
    mov RegionMinY, Aux
    mov Aux, RegionMaxY
    iadd Aux, OffsetY
    mov RegionMaxY, Aux
    mov Aux, RegionHotY
    iadd Aux, OffsetY
    mov RegionHotY, Aux
    
    ; repeat if we did not define all rows
    mov Aux, RegionsInY
    isub Aux, 1
    mov RegionsInY, Aux
    jt Aux, _define_region_matrix_loop_y
    
  ; remove all 9 arguments from the stack
  ; but preserve the return address
  mov Aux, [SP]
  mov [SP+9], Aux
  iadd SP, 9
    
  ; finished, go back
  ret

; remove names for subroutine arguments
%undef RegionMinX
%undef RegionMinY
%undef RegionMaxY
%undef RegionMaxY
%undef RegionHotX
%undef RegionHotY
%undef RegionsInX
%undef RegionsInY
%undef RegionsGap

; remove names for working registers
%undef RegionID
%undef CounterX
%undef OffsetX
%undef OffsetY
%undef WorkingMinX
%undef WorkingMaxX
%undef WorkingHotX
%undef Aux

; end include guard
%endif
