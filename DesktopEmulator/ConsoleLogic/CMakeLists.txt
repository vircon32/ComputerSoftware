# source files to compile library V32ConsoleLogic
add_library(V32ConsoleLogic STATIC
    AuxiliaryFunctions.cpp
    ExternalInterfaces.cpp
    V32Buses.cpp
    V32CartridgeController.cpp
    V32Console.cpp
    V32CPU.cpp
    V32CPUProcessors.cpp
    V32GamepadController.cpp
    V32GPU.cpp
    V32GPUWriters.cpp
    V32Memory.cpp
    V32MemoryCardController.cpp
    V32NullController.cpp
    V32RNG.cpp
    V32SPU.cpp
    V32SPUWriters.cpp
    V32Timer.cpp)
