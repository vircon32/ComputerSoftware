# -----------------------------------------------------
#   Original CMake file written by Alexandre Díaz
#   and later adapted by Carra
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
#   DEFINE THE PROJECT
# -----------------------------------------------------

# Declare the project
# This will make installation create a folder named Vircon32
project("Vircon32" LANGUAGES C CXX)

# Define version
set(PROJECT_VERSION_MAJOR 25)
set(PROJECT_VERSION_MINOR 12)
set(PROJECT_VERSION_PATCH 28)

# Set names for final executables
set(EMULATOR_BINARY_NAME "Vircon32")
set(EDITCONTROLS_BINARY_NAME "EditControls")

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
#   BASIC PROJECT CONFIGURATION
# -----------------------------------------------------

# These general project variables should be cached
set(LIBRARIES_DIR "ExternalLibraries/"
    CACHE PATH "The path to the external libraries.")
set(PROGRAMS_DIR "ExternalPrograms/"
    CACHE PATH "The path to the external programs.")
set(EMULATOR_DIR "Emulator/"
    CACHE PATH "The path to the emulator sources.")
set(CONSOLELOGIC_DIR "ConsoleLogic/"
    CACHE PATH "The path to the core console logic sources.")
set(EDITCONTROLS_DIR "ControlsEditor/"
    CACHE PATH "The path to EditControls sources.")
set(INFRASTRUCTURE_DIR "DesktopInfrastructure/"
    CACHE PATH "The path to desktop infrastructure sources.")
set(DEFINITIONS_DIR "../VirconDefinitions/"
    CACHE PATH "The path to desktop Vircon definitions sources.")
set(DATA_DIR "Data/"
    CACHE PATH "The path to folder containing data files.")
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
#   BUILD FLAGS / CONFIGURATION
# -----------------------------------------------------

# Set compilation flags for C and C++
if(MINGW OR TARGET_OS STREQUAL "linux")
    set(cxx_flags "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wno-unused-parameter")
    set(c_flags "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
elseif(MSVC)
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
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# Library osdialog can target differeng GUI systems (win/mac/gtk2/gtk3/zenity)
if(TARGET_OS STREQUAL "windows")
    set(OSDIALOG_TARGET win CACHE STRING "osdialog target env")
elseif(TARGET_OS STREQUAL "linux")
    # Linux-specific dependency: GTK3 or GTK2 functionality available
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 QUIET gtk+-3.0)
    if (GTK3_FOUND)
        message("--  * GTK3 version (${GTK3_VERSION})")
        set(OSDIALOG_TARGET gtk CACHE STRING "osdialog target env")
        set(GTK "GTK3")
        set(GTK_LIBRARIES ${GTK3_LIBRARIES})
        set(GTK_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS})
    else()
        pkg_check_modules(GTK2 REQUIRED QUIET gtk+-2.0)
        message("--  * GTK2 version (${GTK2_VERSION})")
        set(OSDIALOG_TARGET gtk CACHE STRING "osdialog target env")
        set(GTK "GTK2")
        set(GTK_LIBRARIES ${GTK2_LIBRARIES})
        set(GTK_INCLUDE_DIRS ${GTK2_INCLUDE_DIRS})
    endif()
elseif(TARGET_OS STREQUAL "mac")
    set(OSDIALOG_TARGET mac CACHE STRING "osdialog target env")
    enable_language(OBJC)
endif()

# These are treated as independent (they don't depend on anything else)
find_library(PNG_LIBRARY NAMES png REQUIRED)

# -----------------------------------------------------
#   SHOW BUILD INFORMATION IN PRETTY FORMAT
# -----------------------------------------------------

message(STATUS "******** Emulator ********")

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
show_dependency_status("OPENGL" OPENGL)
show_dependency_status("SDL2" SDL2)
show_dependency_status("PNG" PNG)

# Linux-specific: report on GTK dependency
if(TARGET_OS STREQUAL "linux")
    show_dependency_status("${GTK}" ${GTK})
endif()

# -----------------------------------------------------
#   FOLDERS FOR INCLUDES
# -----------------------------------------------------

# Define folders where compiler should look for includes
set(ALL_INCLUDE_DIRS
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${GTK_INCLUDE_DIRS}
    ${LIBRARIES_DIR}
    ${LIBRARIES_DIR}/glad/include/)
  
include_directories(${ALL_INCLUDE_DIRS})
include_directories(.)

# -----------------------------------------------------
#   LINKED LIBRARIES FILES
# -----------------------------------------------------

# Add external libraries
add_subdirectory(${LIBRARIES_DIR})

# Add project's own libraries
add_subdirectory(${CONSOLELOGIC_DIR})

# Libraries to link with the emulator
set(EMULATOR_LIBS
    osdialog
    imgui
    V32ConsoleLogic
    tinyxml2
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    ${PNG_LIBRARY}
    glad
    ${CMAKE_DL_LIBS})

# Libraries to link with the EditControls tool
set(EDITCONTROLS_LIBS
    osdialog
    imgui
    tinyxml2
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    glad
    ${CMAKE_DL_LIBS})

# -----------------------------------------------------
#   SOURCE FILES
# -----------------------------------------------------

# Source files to compile for the emulator
set(EMULATOR_SRC
    ${EMULATOR_DIR}/AudioOutput.cpp
    ${EMULATOR_DIR}/EmulatorControl.cpp
    ${EMULATOR_DIR}/GamepadsInput.cpp
    ${EMULATOR_DIR}/Globals.cpp
    ${EMULATOR_DIR}/GUI.cpp
    ${EMULATOR_DIR}/Languages.cpp
    ${EMULATOR_DIR}/Main.cpp
    ${EMULATOR_DIR}/Savestates.cpp
    ${EMULATOR_DIR}/Settings.cpp
    ${EMULATOR_DIR}/StopWatch.cpp
    ${EMULATOR_DIR}/Texture.cpp
    ${EMULATOR_DIR}/VideoOutput.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/Logger.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# Source files to compile for the EditControls tool
set(EDITCONTROLS_SRC
    ${EDITCONTROLS_DIR}/Controls.cpp
    ${EDITCONTROLS_DIR}/Globals.cpp
    ${EDITCONTROLS_DIR}/GUI.cpp
    ${EDITCONTROLS_DIR}/Languages.cpp
    ${EDITCONTROLS_DIR}/Main.cpp
    ${EDITCONTROLS_DIR}/OpenGL2DContext.cpp
    ${EDITCONTROLS_DIR}/LoadTexture.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/Logger.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp)

# -----------------------------------------------------
#   EXECUTABLES
# -----------------------------------------------------

# Both emulator and EditControls should be GUI-only applications.
# This setting will prevent creation of an additional console window
# (removed for now on Mac, would be MACOSX_BUNDLE)
if(TARGET_OS STREQUAL "windows")
    set(GUI_TYPE WIN32)
else()
    set(GUI_TYPE "")
endif()

# Define final executable for the emulator
add_executable(${EMULATOR_BINARY_NAME} ${GUI_TYPE} ${EMULATOR_SRC})
set_property(TARGET ${EMULATOR_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# Libraries to link to the emulator executable
target_link_libraries(${EMULATOR_BINARY_NAME} ${EMULATOR_LIBS})

# Define final executable for the EditControls tool
add_executable(${EDITCONTROLS_BINARY_NAME} ${GUI_TYPE} ${EDITCONTROLS_SRC})
set_property(TARGET ${EDITCONTROLS_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# Libraries to link to the EditControls executable
target_link_libraries(${EDITCONTROLS_BINARY_NAME} ${EDITCONTROLS_LIBS})

# On windows both binaries will also need this library
if(TARGET_OS STREQUAL "windows")
    target_link_libraries(${EMULATOR_BINARY_NAME} imm32)
    target_link_libraries(${EDITCONTROLS_BINARY_NAME} imm32)
endif()

# On linux both binaries will also need this set of libraries
if(TARGET_OS STREQUAL "linux")
    target_link_libraries(${EMULATOR_BINARY_NAME} ${GTK_LIBRARIES})
    target_link_libraries(${EDITCONTROLS_BINARY_NAME} ${GTK_LIBRARIES})
endif()

# On mac both binaries will also need this framework
if(TARGET_OS STREQUAL "mac")
    target_link_libraries(${EMULATOR_BINARY_NAME} "-framework AppKit")
    target_link_libraries(${EDITCONTROLS_BINARY_NAME} "-framework AppKit")
endif()

# -----------------------------------------------------
#   ON WINDOWS, CONFIGURE APPLICATION PROPERTIES
# -----------------------------------------------------

if(TARGET_OS STREQUAL "windows")
    # Application properties for the emulator
    add_custom_command(TARGET ${EMULATOR_BINARY_NAME} POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EMULATOR_BINARY_NAME}> --set-icon ${CMAKE_SOURCE_DIR}/${DATA_DIR}/Images/Vircon32Multisize.ico
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EMULATOR_BINARY_NAME}> --set-version-string "ProductName" "Vircon32"
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EMULATOR_BINARY_NAME}> --set-version-string "FileDescription" "Vircon32 console emulator"
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EMULATOR_BINARY_NAME}> --set-version-string "CompanyName" "Javier Carracedo")
    
    # Application properties for the EditControls tool
    add_custom_command(TARGET ${EDITCONTROLS_BINARY_NAME} POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EDITCONTROLS_BINARY_NAME}> --set-icon ${CMAKE_SOURCE_DIR}/${DATA_DIR}/Images/EditControlsMultisize.ico
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EDITCONTROLS_BINARY_NAME}> --set-version-string "ProductName" "EditControls"
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EDITCONTROLS_BINARY_NAME}> --set-version-string "FileDescription" "Input controls editor for Vircon32 emulator"
        COMMAND ${CMAKE_SOURCE_DIR}/${PROGRAMS_DIR}/rcedit.exe $<TARGET_FILE:${EDITCONTROLS_BINARY_NAME}> --set-version-string "CompanyName" "Javier Carracedo")
endif()

# -----------------------------------------------------
#   DEFINE THE INSTALL PROCESS
# -----------------------------------------------------

if(TARGET_OS STREQUAL "windows")
    # Install both binaries
    install(TARGETS ${EMULATOR_BINARY_NAME} ${EDITCONTROLS_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION Emulator)

    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION Emulator)
    
    # On Windows, copy the needed runtime files (DLLs)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${RUNTIME_DIR}/
        DESTINATION Emulator)
else()
    # Install both binaries
    install(TARGETS ${EMULATOR_BINARY_NAME} ${EDITCONTROLS_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION ${CMAKE_PROJECT_NAME}/Emulator)

    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION ${CMAKE_PROJECT_NAME}/Emulator)
endif()

# Extra steps to configure the application on Linux
if(TARGET_OS STREQUAL "linux")
    # Install desktop entry
    install(FILES Resources/Linux/Vircon32.desktop DESTINATION /usr/share/applications )

    # Install icon
    install(FILES Resources/Linux/Vircon32.svg DESTINATION /usr/share/pixmaps )
    install(FILES Resources/Linux/Vircon32.svg DESTINATION /usr/share/icons/hicolor/scalable/apps/ )
endif()

# On Linux or Mac we will need to set install permissions
if(TARGET_OS STREQUAL "linux" OR TARGET_OS STREQUAL "mac")
    # Allow emulator to create the debug log
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator)")
    
    # Allow emulator and editcontrols to save settings
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator/Config-Controls.xml)")
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator/Config-Settings.xml)")
    
    # Allow emulator to create memory cards, savestates and screenshots
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator/Cards)")
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator/Savestates)")
    install(CODE "execute_process(COMMAND chmod 777 ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}/Emulator/Screenshots)")
endif()

# -----------------------------------------------------
#   DEFINE THE PACKAGE CREATION WITH CPACK
# -----------------------------------------------------

if(TARGET_OS STREQUAL "linux")

    # define install directory
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
    set(CPACK_INSTALL_PREFIX "/usr/local")

    # define base package info
    set(CPACK_PACKAGE_NAME "vircon32-emulator")
    set(CPACK_PACKAGE_VERSION "25.12.28")
    set(CPACK_PACKAGE_VENDOR "Carra")
    set(CPACK_PACKAGE_MAINTAINER "Carra")
    set(CPACK_PACKAGE_LICENSE "BSD-3-Clause")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop emulator for Vircon32 console")
    set(CPACK_PACKAGE_DESCRIPTION "This is the official Vircon32 desktop emulator for the Debian Linux family. Vircon32 is an open source, 32-bit virtual game console")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.vircon32.com")
    
    # Set dependencies and package specific configuration
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Carra")
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")

    set(CPACK_RPM_PACKAGE_AUTOREQ ON)
    set(CPACK_RPM_PACKAGE_AUTOPROV ON)
    set(CPACK_RPM_PACKAGE_NAME "vircon32-emulator")
    set(CPACK_RPM_PACKAGE_LICENSE "BSD-3-Clause")
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")

    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    # finally this reads all CPack variables
    include(CPack)

endif()
