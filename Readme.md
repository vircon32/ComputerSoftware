# Vircon32: Computer software

This is a general repository containing source code related to Vircon32 implementation, this is, software that does NOT run in the console itself but in an external computer or device.

This will include:
- Emulator for desktop PC
- Development tools (assembler, C compiler, file importers)
- Any future emulators (for Raspberry Pi, etc)
- Other tools (texture region editor, etc)
- The C compiler's standard library
  (it DOES run on the console, but it is part of the compiler)

However, it does not include console software such as the following:
- Games and demos
- Examples and tutorials
- The BIOS

Most of these projects are written in C++ version 2011, and use portable libraries (SDL, OpenGL, OpenAL) along with CMake in an attempt to support multiplatform compilation.

----------------------------------
### Where to learn about Vircon32?

If you are not familiar with Vircon32, a 32-bit virtual game console, you can visit its website [www.vircon32.com](http://www.vircon32.com "Vircon32 Homepage") for more information. There you will also find everything you may need to try out the console and to make games for it.

--------------------------------------------
### How to build these programs from source?

All C++ projects in this repository are prepared to compile using CMake. The CMake scripts do try to support (at least) building under Windows, Linux and Mac. Note that, under Windows, CMake defaults to building with Visual C++, but this is currently untested. To instruct CMake to build with MSYS2+MinGW, open a MinGW32 console and go to your desired build directory. Then use the command: `cmake -G 'MSYS Makefiles' <project's root CMake folder>`. For example, if you are at the project's root folder (i.e. the folder with the main CMakeLists.txt file) you could write these commands to make the build in a temporary folder and then install the software:

##### WINDOWS

1. `mkdir build` to create a folder for your build
2. `cd build` to enter the build folder
3. `cmake -G 'MSYS Makefiles' ..` to have CMake configure the project for MinGW 
4. `make` to build the project
5. `cmake --install .` to install the built software

##### LINUX (DesktopEmulator)

1. `mkdir build` to create a folder for your build
2. `cd build` to enter the build folder
3. `cmake -DCMAKE_INSTALL_PREFIX:PATH=./dist -DINSTALL_DESKTOP_FILES:BOOL=OFF ../DesktopEmulator` to have CMake configure the project for MinGW 
4. `make` to build the project
5. `make install` to install the built software

Note that you can avoid set `CMAKE_INSTALL_PREFIX` and `INSTALL_DESKTOP_FILES` to make a local installation (~/.local). You
can install globally setting `INSTALL_DESKTOP_FILES_GLOBAL` to `ON` (you will need execute installation with sudo).

At that point, if no errors happened, your programs should be installed and able to run correctly. Install folder will be named "Vircon32" and placed in your system's default program path. You can then delete the temporary 'build' directory: it is no longer needed.

--------------------------------------------
### Dependencies on other libraries

To correctly build the software in this repository you will need to have at least the following libraries available in your build system. Note that these may in turn have dependencies themselves, which you will also need.

##### General purpose libraries:
* SDL2
* TinyXML2

##### Video libraries:
* SDL2_image
* OpenGL
* GLUT
* LibPNG

##### Audio libraries:
* OpenAL
* ALUT / FreeALUT

##### Build system
* cmake
* pkg-config

These programs also use 3 other libraries that are already included in the sources as external libraries: osdialog, imgui and glad.
