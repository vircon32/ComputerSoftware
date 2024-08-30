============================================================
             README file for Vircon32 emulator
            (version 24.8.30) written by Carra
============================================================

What is this?

    This is an emulator of Vircon32, a 32-bit virtual console,
    for PC. It will allow you to play Vircon32 games on your
    computer.

    The present version is considered finished, save for any
    bugs fixes that may be needed in the future. All console
    features are already supported, and all planned extra
    functions for the emulator (settings, GUI, etc) are done.

------------------------------------------------------------

Installing 
    
    Depending on your operating system, the process to prepare
    the emulator for use will be different:
    
    Windows:
    --------
    Just extract the ZIP into any folder. All needed DLL
    libraries are already included.
      
    Linux:
    --------
    Install the DEB or RPM package (depending on your system)
    using the corresponding commands:
      DEB: (1) Install package: sudo dpkg -i <paquete>
           (2) Install dependencies: sudo apt-get install -f
      RPM: Install package along with its dependencies:
           sudo yum localinstall <paquete>
              
    MacOS:
    --------
    First extract the ZIP into any folder with write permission.
    To install dependencies, the Hombrew package manager is used
    (it can be installed from its website). The commands needed
    to install the dependencies are:
      brew install sdl2
      brew install sdl2_image
      brew install openal-soft
      brew install freealut
      brew install tinyxml2
    
------------------------------------------------------------

The program window

    Graphic interface is normally hidden, but it will be shown
    when mouse is over the emulator window. It can also be made
    to show manually by pressing the escape key.

    Use the mouse to navigate menus and use their options.

------------------------------------------------------------

How to load games

    Vircon32 is an emulator, so for it to work you will need to
    insert a virtual cartridge, also called a ROM file, and then
    power on the console.

    In this console the cartridge slot becomes locked when the
    console is powered on, so to release a cartridge or replace
    it with another you will need to power off the console first.

    You can also open a game by just double-clicking a *.v32
    ROM file and, in the "Open with..." dialog, selecting the
    Vircon32 emulator as program to open it.
    
------------------------------------------------------------

Controls

    By default only gamepad 1 is connected and it is mapped to
    the keyboard as follows:

      - D-Pad: Direction arrow keys
      - Buttons L,R: Keys 'Q','W'
      - Buttons Y,X: Keys 'A','S'
      - Buttons B,A: Keys 'Z','X'
      - Button Start: Key 'Return'

    The emulator also supports the use of one or more joysticks
    (since this console has 4 gamepad ports). Keyboard and
    joystick controls can be edited using a second program
    included with this emulator, called EditControls. Read the
    help in EditControls to learn how to configure your devices.

------------------------------------------------------------

How to use memory cards

    When some Vircon32 cartridge needs to save/load data between
    sessions, it will need a memory card to preserve any saved
    information.

    There are 2 modes you can choose to manage memory cards. You
    can choose which one to use from the options menu.
    
    Automatic (default option):
    
      The emulator automatically creates a new memory card for
      each game we open. It saves them in subfolder "Cards"
      within the emulador folder.
      
      This mode is the same one used by the Vircon32 core for
      RetroArch. Many other emulators also work like this.
    
    Manual:
    
      The emulator will not create or use cards automatically.
      Instead, from the memory card menu you can create new
      cards and decide when to load and remove them.
      
      This mode mirrors the way you would use a physical
      console, handling the cards and cartridges ourselves.
    
    If you use manual mode, be careful to use the right memory
    card for each cartridge! Each game should check that the
    data in the card does not belong to other cartridges, but
    there is no guarantee that it actually will.

------------------------------------------------------------

How to use savestates
    
    Savestates allow you to save the whole state of the console
    at any moment while playing a game, and then load it at a
    later time. This will let you continue playing later, even
    if the game itself does not allow you to save the game.
    
    There are 4 savestate slots for each game. By default you
    use slot 1 but you can select another slot using the menus.
    When you load and save state be careful to do it from the
    correct slot, since these actions cannot be undone!
    
    Note that savestates do not save the screen contents,
    which are persistent on Vircon32. This is done on purpose
    to reduce savestate size and complexity. Most games will
    update the screen every frame, but in a few cases your
    screen will not immediately reflect the changes after
    loading a savestate. Especially if you saved a state
    during menus or screen transitions.
    
------------------------------------------------------------

What's new in version 24.8.30?
    
  - Added support for savestates.

------------------------------------------------------------

License

    This program is free and open source. It is offered under
    the 3-Clause BSD License, which full text is the following:
    
    Copyright 2021-2024 Carra.
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
