================================================
   PC development tools for Vircon32 console
   (version 23.1.16). README written by Carra
================================================


What is this?

    This is a set of tools that allow you to create Vircon32
    programs from your PC, by writing C or assembly programs.
    All the programs included here are command-line tools.
    
------------------------------------------------------------

Installing 
    
    These tools are installed following a process similar to
    the one used for installing the emulator (which you will
    probably need as well). The exact steps will depend on
    your operating system.
    
    There is, however, one additional step needed after
    installing: you will need to ensure that the installed
    programs are accessible to the environment by adding the
    installation folder to the corresponding system paths.
    
------------------------------------------------------------

Included programs 
  
  - "compile": a C compiler that allows you to compile your
    C programs for Vircon32 into assembly. It also includes
    the headers for its standard library.
    
  - "assemble": an assembler that allows you to convert your
    assembly programs for Vircon32 into machine code.
  
  - "png2vircon": a program that allows you to convert PNG
    images into the native textures used by Vircon32.
  
  - "wav2vircon": a program that allows you to convert WAV
    sounds into the native sounds used by Vircon32
  
  - "tiled2vircon": a program that allows you to convert
    tilemaps made in the Tiled editor into 2D arrays that
    can be used natively as part of Vircon32 programs.
  
  - "packrom": this program that allows you to pack the
    binaries, images and sound of a same Vircon32 program
    into a single file, that will be executable from any
    Vircon32 emulator or implementation.
    
------------------------------------------------------------

What's new in version 23.1.16?
  
  - Added support for partial declarations in structures,
    unions and function prototypes.
  - Added support for SPU channel position port.
  - Added specific error messages in some non-supported C
    features to be more helpful.
  - Corrected wrong compilation error when maximum warnings
    were reached.
  - Add checks to prevent use of void values, and avoid
    dereferencing void pointers.
  - Fixed bad use of R0 in some expressions with nested
    function calls.
  - Fixed bug when initializing pointers to NULL.
  - Corrected error in ftoa implementation, that provided
    wrong results on decimal parts with initial zeroes.

------------------------------------------------------------

License

    These programs are free and open source. They are offered
    under the 3-Clause BSD License, which full text is the
    following:
    
    Copyright 2021-2023 Carra.
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or
    without modification, are permitted provided that the
    following conditions are met:
    
    1. Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.
    
    2. Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.
    
    3. Neither the name of the copyright holder nor the names of
    its contributors may be used to endorse or promote products
    derived from this software without specific prior written
    permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
