; the binary will always begin executing from the
; first word so we need to jump to our actual program
jmp _main

; ------------ DATA TYPES ---------------

; define data values: literal integers
_literal_ints:
  integer 1, -3, 0xA, 'H', '\x41'

; true/false are just read as integers 1/0
_literal_booleans:
  integer true, false

; define data values: literal floats
; (note that exponential notation is not supported)
_literal_floats:
  float  7.0, -0.9, 1.1

; define data values: strings
_message:
  string "hello \"world\""  ; uses escaping to write double quotes

; define pointers to other addresses
_pointers:
  pointer _message, _literal_ints

; we can also embed the content of a file
; and access its first word from the label
_data:
  datafile "DataFile.dat"

; ------------ DIRECTIVES ---------------

; include auxiliary subs in another file
%include "IncludeAux.asm"

; some custom definitions
%define reg_0 R0
%define pi 3.1416

; conditional compilation
%define DEBUG

%ifdef DEBUG
  ; this is just a warning message
  %warning "DEBUG is defined"  
%endif

%ifndef DEBUG
  ; this will stop the assembly
  %error "DEBUG is not defined!"
%else
  ; definitions can be more than 1 token, but only span 1 line
  ; (but do NOT surround them in parentheses like in C!!)
  %define LOADZERO  mov R0, 0
%endif

; remove definitions when no longer needed
%undef DEBUG

; ------------ THE PROGRAM ---------------

; does nothing
_main:
  LOADZERO
  hlt
