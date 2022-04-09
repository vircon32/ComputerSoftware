cmake_minimum_required(VERSION 3.19)

project( VirconDefinitions )

add_library( VirconDefinitions STATIC
    VirconDefinitions.cpp
    VirconEnumerations.cpp
    VirconROMFormat.cpp )