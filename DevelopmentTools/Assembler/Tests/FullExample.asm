; define data values: literal integers
_literals_a:
    integer 1, -3, 0xA
    
; define data values: literal floats
_literals_b:
    float  7.0, -0.9, 1.1

; define data values: strings
_message:
  string "hola"

; define pointers to other addresses
_pointers:
  pointers _message, _literals_a

; we can also embed the content of a file
; and access its first word from the label
_data:
  datafile "DataFile.dat"

; some custom definitions
%define reg_0 R0
%define pi 3.1416
