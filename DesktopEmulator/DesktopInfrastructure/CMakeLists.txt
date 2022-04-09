cmake_minimum_required(VERSION 3.19)

project( DesktopInfrastructure )

add_library( DesktopInfrastructure STATIC
    Definitions.cpp
    FilePaths.cpp
    LogStream.cpp
    OpenGL2D.cpp
    StopWatch.cpp
    StringFunctions.cpp
    Texture.cpp
    Vector2D.cpp
    Video.cpp )