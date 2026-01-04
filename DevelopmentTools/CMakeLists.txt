# -----------------------------------------------------
#   CMake file written by Carra
#   based on the original work by Alexandre Díaz
# -----------------------------------------------------

# minimum version of CMake that can parse this file
cmake_minimum_required(VERSION 2.8.12...3.19.1)

# configure some flags for compatibility across CMake versions
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW) # Ignore Quoted Arguments
endif()
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW) # Ignore Legacy GL
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # Root Variables
endif()

# -----------------------------------------------------
#   BASIC PROJECT CONFIGURATION
# -----------------------------------------------------

# These general project variables should be cached
set(LIBRARIES_DIR "ExternalLibraries/"
    CACHE PATH "The path to the external libraries.")
set(C_COMPILER_DIR "CCompiler/"
    CACHE PATH "The path to the C compiler sources.")
set(ASSEMBLER_DIR "Assembler/"
    CACHE PATH "The path to the assembler sources.")
set(ROM_PACKER_DIR "RomPacker/"
    CACHE PATH "The path to the ROM packer sources.")
set(PNG_CONVERTER_DIR "PNG2Vircon/"
    CACHE PATH "The path to the PNG converter sources.")
set(WAV_CONVERTER_DIR "WAV2Vircon/"
    CACHE PATH "The path to the WAV converter sources.")
set(TILED_CONVERTER_DIR "Tiled2Vircon/"
    CACHE PATH "The path to the Tiled converter sources.")
set(PNG_JOINER_DIR "PNGJoiner/"
    CACHE PATH "The path to the PNG joiner sources.")
set(DISASSEMBLER_DIR "Disassembler/"
    CACHE PATH "The path to the disassembler sources.")
set(ROM_UNPACKER_DIR "RomUnpacker/"
    CACHE PATH "The path to the ROM unpacker sources.")
set(INFRASTRUCTURE_DIR "DevToolsInfrastructure/"
    CACHE PATH "The path to desktop infrastructure sources.")
set(DEFINITIONS_DIR "../VirconDefinitions/"
    CACHE PATH "The path to desktop Vircon definitions sources.")
set(DATA_DIR "Data/"
    CACHE PATH "The path to data files to add when installing.")
set(RUNTIME_DIR "Runtime/"
    CACHE PATH "The path to the runtime files (DLLs).")

# Configure find_* commands to never try to find Mac frameworks, only packages
set(CMAKE_FIND_FRAMEWORK CACHE STRING "NEVER")

# By default, project configuration will be Release
# (must be done before project() statement)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# -----------------------------------------------------
#   DEFINE THE PROJECT
# -----------------------------------------------------

# Declare the project
project("Vircon32")

# Define version
set(PROJECT_VERSION_MAJOR 25)
set(PROJECT_VERSION_MINOR 12)
set(PROJECT_VERSION_PATCH 28)

# Set names for final executables
set(C_COMPILER_BINARY_NAME "compile")
set(ASSEMBLER_BINARY_NAME "assemble")
set(ROM_PACKER_BINARY_NAME "packrom")
set(PNG_CONVERTER_BINARY_NAME "png2vircon")
set(WAV_CONVERTER_BINARY_NAME "wav2vircon")
set(TILED_CONVERTER_BINARY_NAME "tiled2vircon")
set(PNG_JOINER_BINARY_NAME "joinpngs")

# Set names for reverse tools executables
set(DISASSEMBLER_BINARY_NAME "disassemble")
set(ROM_UNPACKER_BINARY_NAME "unpackrom")
set(PNG_EXTRACTOR_BINARY_NAME "vircon2png")
set(WAV_EXTRACTOR_BINARY_NAME "vircon2wav")

# -----------------------------------------------------
#   IDENTIFY HOST ENVIRONMENT
# -----------------------------------------------------

# Detect architecture bits
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_BITS "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_BITS "32")
endif()

# Detect operating system
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TARGET_OS "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TARGET_OS "mac")
endif()

# -----------------------------------------------------
#   BUILD FLAGS / CONFIGURATION
# -----------------------------------------------------

# Set compilation flags for C and C++
if(MINGW OR TARGET_OS STREQUAL "linux")
    set(cxx_flags "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    set(c_flags "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
elseif (MSVC)
    set(cxx_flags "${CMAKE_CXX_FLAGS} /W3 /EHsc /MP /GS /wd4267 /wd4244")
    set(c_flags "${CMAKE_C_FLAGS} /W3 /MP /GS /wd4267 /wd4244")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_FLAGS "${cxx_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)
set(CMAKE_C_FLAGS "${c_flags}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)

# Mark executables as debug (*_d) when debug build is selected
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_CFG_INTDIR STREQUAL "Debug" OR CMAKE_CFG_INTDIR STREQUAL "RelWithDebInfo")
	set(IS_DEBUG TRUE)
	set(CMAKE_DEBUG_POSTFIX "_d")
	if(TARGET_OS STREQUAL "windows")
		set(CMAKE_EXECUTABLE_SUFFIX "_d.exe")
	else()
		set(CMAKE_EXECUTABLE_SUFFIX "_d")
	endif()
else()
	set(IS_DEBUG FALSE)
endif()

# -----------------------------------------------------
#   FINDING ALL PROJECT DEPENDENCIES
# -----------------------------------------------------

# Add this folder to use the FindXXX.cmake files
# (they tell CMake how to find specific dependencies)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

# For these depencencies, find everything they need too
find_package(SDL2 REQUIRED)

# These are treated as independent (they don't depend on anything else)
find_library(PNG_LIBRARY NAMES png REQUIRED)

# -----------------------------------------------------
#   SHOW BUILD INFORMATION IN PRETTY FORMAT
# -----------------------------------------------------

message(STATUS "******** DevTools ********")

# Show basic build properties
if(NOT TARGET_OS STREQUAL "mac")
    message(STATUS "Target OS: ${TARGET_OS} ${TARGET_BITS}bit")
else()
    message(STATUS "Target OS: ${TARGET_OS} ${TARGET_BITS}bit (SDK: ${CMAKE_OSX_SYSROOT})")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# This function shows the status for a dependency in pretty format
function(show_dependency_status OUTPUT_NAME NAME)
    if(${NAME}_FOUND OR ${NAME}_LIBRARY)
        if(${NAME}_BUNDLED)
            message(STATUS " * ${OUTPUT_NAME} not found (using bundled version)")
        else()
            message(STATUS " * ${OUTPUT_NAME} found")
        endif()
    else()
        message(STATUS " * ${OUTPUT_NAME} not found")
    endif()
endfunction()

# Now use the function to show status of all dependencies
message(STATUS "System Dependencies:")
show_dependency_status("SDL2" SDL2)
show_dependency_status("LIBPNG" PNG)

# -----------------------------------------------------
#   FOLDERS FOR INCLUDES
# -----------------------------------------------------

# Define folders where compiler should look for includes
set(ALL_INCLUDE_DIRS
    ${SDL2_INCLUDE_DIR}
    ${C_COMPILER_DIR}
    ${ASSEMBLER_DIR}
    ${PNG_CONVERTER_DIR}
    ${WAV_CONVERTER_DIR}
    ${TILED_CONVERTER_DIR}
    ${PNG_JOINER_DIR}
    ${ROM_PACKER_DIR}
    ${DISASSEMBLER_DIR}
    ${ROM_UNPACKER_DIR}
    ${INFRASTRUCTURE_DIR}
    ${DEFINITIONS_DIR}
    ${LIBRARIES_DIR})

include_directories(${ALL_INCLUDE_DIRS})

# -----------------------------------------------------
#   LINKED LIBRARIES FILES (BUILD TOOLS)
# -----------------------------------------------------

# Add external libraries
add_subdirectory(${LIBRARIES_DIR})

# Libraries to link with the C compiler
set(C_COMPILER_LIBS
    ${SDL2_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the assembler
set(ASSEMBLER_LIBS
    ${SDL2_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the ROM packer
set(ROM_PACKER_LIBS
    tinyxml2
    ${CMAKE_DL_LIBS})

# Libraries to link with the PNG converter
set(PNG_CONVERTER_LIBS
    ${PNG_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the WAV converter
set(WAV_CONVERTER_LIBS
    ${SDL2_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the Tiled converter
set(TILED_CONVERTER_LIBS
    tinyxml2
    ${CMAKE_DL_LIBS})

# Libraries to link with the PNG joiner
set(PNG_JOINER_LIBS
    ${PNG_LIBRARY}
    ${CMAKE_DL_LIBS})

# -----------------------------------------------------
#   LINKED LIBRARIES FILES (REVERSE TOOLS)
# -----------------------------------------------------

# Libraries to link with the disassembler
set(DISASSEMBLER_LIBS
    ${CMAKE_DL_LIBS})

# Libraries to link with the ROM unpacker
set(ROM_UNPACKER_LIBS
    ${CMAKE_DL_LIBS})

# Libraries to link with the PNG extractor
set(PNG_EXTRACTOR_LIBS
    ${PNG_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the WAV extractor
set(WAV_EXTRACTOR_LIBS
    ${CMAKE_DL_LIBS})

# -----------------------------------------------------
#   SOURCE FILES (BUILD TOOLS)
# -----------------------------------------------------

# Source files to compile for the C compiler
set(C_COMPILER_SRC
    ${C_COMPILER_DIR}/CNodes.cpp
    ${C_COMPILER_DIR}/CTokens.cpp
    ${C_COMPILER_DIR}/CheckBinaryOperations.cpp
    ${C_COMPILER_DIR}/CheckNodes.cpp
    ${C_COMPILER_DIR}/CheckUnaryOperations.cpp
    ${C_COMPILER_DIR}/CompilerInfrastructure.cpp
    ${C_COMPILER_DIR}/DataTypes.cpp
    ${C_COMPILER_DIR}/DebugInfo.cpp
    ${C_COMPILER_DIR}/EmitBinaryOperationNodes.cpp
    ${C_COMPILER_DIR}/EmitExpressionNodes.cpp
    ${C_COMPILER_DIR}/EmitNonExpressionNodes.cpp
    ${C_COMPILER_DIR}/EmitUnaryOperationNodes.cpp
    ${C_COMPILER_DIR}/Globals.cpp
    ${C_COMPILER_DIR}/Main.cpp
    ${C_COMPILER_DIR}/MemoryPlacement.cpp
    ${C_COMPILER_DIR}/Operators.cpp
    ${C_COMPILER_DIR}/RegisterAllocation.cpp
    ${C_COMPILER_DIR}/SourceLocation.cpp
    ${C_COMPILER_DIR}/StaticValue.cpp
    ${C_COMPILER_DIR}/VirconCAnalyzer.cpp
    ${C_COMPILER_DIR}/VirconCEmitter.cpp
    ${C_COMPILER_DIR}/VirconCLexer.cpp
    ${C_COMPILER_DIR}/VirconCParser.cpp
    ${C_COMPILER_DIR}/VirconCPreprocessor.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/EnumStringConversions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the assembler
set(ASSEMBLER_SRC
    ${ASSEMBLER_DIR}/ASMEmitFunctions.cpp
    ${ASSEMBLER_DIR}/ASTNodes.cpp
    ${ASSEMBLER_DIR}/DebugInfo.cpp
    ${ASSEMBLER_DIR}/Globals.cpp
    ${ASSEMBLER_DIR}/Main.cpp
    ${ASSEMBLER_DIR}/SourceLocation.cpp
    ${ASSEMBLER_DIR}/Tokens.cpp
    ${ASSEMBLER_DIR}/VirconASMEmitter.cpp
    ${ASSEMBLER_DIR}/VirconASMLexer.cpp
    ${ASSEMBLER_DIR}/VirconASMParser.cpp
    ${ASSEMBLER_DIR}/VirconASMPreprocessor.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/EnumStringConversions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the ROM packer
set(ROM_PACKER_SRC
    ${ROM_PACKER_DIR}/Main.cpp
    ${ROM_PACKER_DIR}/RomDefinition.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/FileSignatures.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the PNG converter
set(PNG_CONVERTER_SRC
    ${PNG_CONVERTER_DIR}/png2vircon.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the WAV converter
set(WAV_CONVERTER_SRC
    ${WAV_CONVERTER_DIR}/wav2vircon.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp)

# Source files to compile for the Tiled converter
set(TILED_CONVERTER_SRC
    ${TILED_CONVERTER_DIR}/Tiled2Vircon.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp)

# Source files to compile for the PNG joiner
set(PNG_JOINER_SRC
    ${PNG_JOINER_DIR}/Globals.cpp
    ${PNG_JOINER_DIR}/PNGImage.cpp
    ${PNG_JOINER_DIR}/PNGJoiner.cpp
    ${PNG_JOINER_DIR}/RectangleNode.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# -----------------------------------------------------
#   SOURCE FILES (REVERSE TOOLS)
# -----------------------------------------------------

# Source files to compile for the disassembler
set(DISASSEMBLER_SRC
    ${DISASSEMBLER_DIR}/Globals.cpp
    ${DISASSEMBLER_DIR}/Main.cpp
    ${DISASSEMBLER_DIR}/OperandWriters.cpp
    ${DISASSEMBLER_DIR}/VirconDisassembler.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/EnumStringConversions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/FileSignatures.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the ROM unpacker
set(ROM_UNPACKER_SRC
    ${ROM_UNPACKER_DIR}/Main.cpp
    ${ROM_UNPACKER_DIR}/RomDefinition.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/FileSignatures.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the PNG extractor
set(PNG_EXTRACTOR_SRC
    ${PNG_CONVERTER_DIR}/vircon2png.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/FileSignatures.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the WAV extractor
set(WAV_EXTRACTOR_SRC
    ${WAV_CONVERTER_DIR}/vircon2wav.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/FileSignatures.cpp)

# -----------------------------------------------------
#   EXECUTABLES (BUILD TOOLS)
# -----------------------------------------------------

# Define all final executables for build tools
add_executable(${C_COMPILER_BINARY_NAME} ${C_COMPILER_SRC})
set_property(TARGET ${C_COMPILER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${ASSEMBLER_BINARY_NAME} ${ASSEMBLER_SRC})
set_property(TARGET ${ASSEMBLER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${ROM_PACKER_BINARY_NAME} ${ROM_PACKER_SRC})
set_property(TARGET ${ROM_PACKER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${PNG_CONVERTER_BINARY_NAME} ${PNG_CONVERTER_SRC})
set_property(TARGET ${PNG_CONVERTER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${WAV_CONVERTER_BINARY_NAME} ${WAV_CONVERTER_SRC})
set_property(TARGET ${WAV_CONVERTER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${TILED_CONVERTER_BINARY_NAME} ${TILED_CONVERTER_SRC})
set_property(TARGET ${TILED_CONVERTER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# this tool needs C++17 for a portable way to iterate over folders
add_executable(${PNG_JOINER_BINARY_NAME} ${PNG_JOINER_SRC})
set_property(TARGET ${PNG_JOINER_BINARY_NAME} PROPERTY CXX_STANDARD 17)

# Libraries to link to the C compiler executables
target_link_libraries(${C_COMPILER_BINARY_NAME} ${C_COMPILER_LIBS})
target_link_libraries(${ASSEMBLER_BINARY_NAME} ${ASSEMBLER_LIBS})
target_link_libraries(${ROM_PACKER_BINARY_NAME} ${ROM_PACKER_LIBS})
target_link_libraries(${PNG_CONVERTER_BINARY_NAME} ${PNG_CONVERTER_LIBS})
target_link_libraries(${WAV_CONVERTER_BINARY_NAME} ${WAV_CONVERTER_LIBS})
target_link_libraries(${TILED_CONVERTER_BINARY_NAME} ${TILED_CONVERTER_LIBS})
target_link_libraries(${PNG_JOINER_BINARY_NAME} ${PNG_JOINER_LIBS})

# -----------------------------------------------------
#   EXECUTABLES (REVERSE TOOLS)
# -----------------------------------------------------

# Define all final executables for reverse tools
add_executable(${DISASSEMBLER_BINARY_NAME} ${DISASSEMBLER_SRC})
set_property(TARGET ${DISASSEMBLER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${ROM_UNPACKER_BINARY_NAME} ${ROM_UNPACKER_SRC})
set_property(TARGET ${ROM_UNPACKER_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${PNG_EXTRACTOR_BINARY_NAME} ${PNG_EXTRACTOR_SRC})
set_property(TARGET ${PNG_EXTRACTOR_BINARY_NAME} PROPERTY CXX_STANDARD 11)

add_executable(${WAV_EXTRACTOR_BINARY_NAME} ${WAV_EXTRACTOR_SRC})
set_property(TARGET ${WAV_EXTRACTOR_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# Libraries to link to the C compiler executables
target_link_libraries(${DISASSEMBLER_BINARY_NAME} ${DISASSEMBLER_LIBS})
target_link_libraries(${ROM_UNPACKER_BINARY_NAME} ${ROM_UNPACKER_LIBS})
target_link_libraries(${PNG_EXTRACTOR_BINARY_NAME} ${PNG_EXTRACTOR_LIBS})
target_link_libraries(${WAV_EXTRACTOR_BINARY_NAME} ${WAV_EXTRACTOR_LIBS})

# -----------------------------------------------------
#   DEFINE THE INSTALL PROCESS
# -----------------------------------------------------

if(TARGET_OS STREQUAL "windows")
    # Install all binaries
    install(TARGETS
        ${C_COMPILER_BINARY_NAME}
        ${ASSEMBLER_BINARY_NAME}
        ${ROM_PACKER_BINARY_NAME}
        ${PNG_CONVERTER_BINARY_NAME}
        ${WAV_CONVERTER_BINARY_NAME}
        ${TILED_CONVERTER_BINARY_NAME}
        ${PNG_JOINER_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION DevTools)
    
    # Install all binaries for reverse tools
    install(TARGETS
        ${DISASSEMBLER_BINARY_NAME}
        ${ROM_UNPACKER_BINARY_NAME}
        ${PNG_EXTRACTOR_BINARY_NAME}
        ${WAV_EXTRACTOR_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION DevTools)
    
    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION DevTools)
    
    # On Windows, copy the needed runtime files (DLLs)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${RUNTIME_DIR}/
        DESTINATION DevTools)
else()
    # Install all binaries for build tools
    install(TARGETS
        ${C_COMPILER_BINARY_NAME}
        ${ASSEMBLER_BINARY_NAME}
        ${ROM_PACKER_BINARY_NAME}
        ${PNG_CONVERTER_BINARY_NAME}
        ${WAV_CONVERTER_BINARY_NAME}
        ${TILED_CONVERTER_BINARY_NAME}
        ${PNG_JOINER_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION ${CMAKE_PROJECT_NAME}/DevTools)
    
    # Install all binaries for reverse tools
    install(TARGETS
        ${DISASSEMBLER_BINARY_NAME}
        ${ROM_UNPACKER_BINARY_NAME}
        ${PNG_EXTRACTOR_BINARY_NAME}
        ${WAV_EXTRACTOR_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION ${CMAKE_PROJECT_NAME}/DevTools)
    
    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION ${CMAKE_PROJECT_NAME}/DevTools)
endif()

# On Linux or Mac we will need to set install permissions
if(TARGET_OS STREQUAL "linux" OR TARGET_OS STREQUAL "mac")
    # Allow compiler and assembler to create logs
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/DevTools)")
endif()

# -----------------------------------------------------
#   DEFINE THE PACKAGE CREATION WITH CPACK
# -----------------------------------------------------

if(TARGET_OS STREQUAL "linux")

    # define install directory
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
    set(CPACK_INSTALL_PREFIX "/usr/local")

    # define base package info
    set(CPACK_PACKAGE_NAME "vircon32-devtools")
    set(CPACK_PACKAGE_VERSION "25.12.28")
    set(CPACK_PACKAGE_VENDOR "Carra")
    set(CPACK_PACKAGE_MAINTAINER "Carra")
    set(CPACK_PACKAGE_LICENSE "BSD-3-Clause")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Development tools for Vircon32 console")
    set(CPACK_PACKAGE_DESCRIPTION "These are the official development tools for Vircon32, for the Debian Linux family. Vircon32 is an open source, 32-bit virtual game console")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.vircon32.com")

    # Set dependencies and package specific configuration
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Carra")
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")

    set(CPACK_RPM_PACKAGE_AUTOREQ ON)
    set(CPACK_RPM_PACKAGE_AUTOPROV ON)
    set(CPACK_RPM_PACKAGE_NAME "vircon32-devtools")
    set(CPACK_RPM_PACKAGE_LICENSE "BSD-3-Clause")
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")

    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    
    # finally this reads all CPack variables
    include(CPack)

endif()
