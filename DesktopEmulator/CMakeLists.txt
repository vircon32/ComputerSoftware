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
set(PROJECT_VERSION_MAJOR 22)
set(PROJECT_VERSION_MINOR 6)
set(PROJECT_VERSION_PATCH 4)

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

# Library osdialog can target differeng GUI systems (win/mac/gtk2/gtk3/zenity)
if(TARGET_OS STREQUAL "windows")
    set(OSDIALOG_TARGET win CACHE STRING "osdialog target env")
elseif(TARGET_OS STREQUAL "linux")
    set(OSDIALOG_TARGET gtk2 CACHE STRING "osdialog target env")
elseif(TARGET_OS STREQUAL "mac")
    set(OSDIALOG_TARGET mac CACHE STRING "osdialog target env")
    enable_language(OBJC)
endif()

# These general project variables should be cached
set(LIBRARIES_DIR "ExternalLibraries/"
    CACHE PATH "The path to the external libraries.")
set(PROGRAMS_DIR "ExternalPrograms/"
    CACHE PATH "The path to the external programs.")
set(EMULATOR_DIR "Emulator/"
    CACHE PATH "The path to the emulator sources.")
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

# Configure some legacy CMake behavior ("policies")
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)	# Ignore Quoted Arguments
endif()
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW) # Ignore Legacy GL
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # Root Variables
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
find_package(GLUT REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenAL REQUIRED)

# On Linux we will need to find GTK too
if(TARGET_OS STREQUAL "linux")
    find_package(GTK2 COMPONENTS gtk REQUIRED)
endif()

# These are treated as independent (they don't depend on anything else)
find_library(PNG_LIBRARY NAMES png REQUIRED)
find_library(FREEALUT_LIBRARY NAMES freealut alut REQUIRED)
find_library(TINYXML2_LIBRARY tinyxml2 REQUIRED)

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
show_dependency_status("GLUT" GLUT)
show_dependency_status("SDL2" SDL2)
show_dependency_status("SDL2_image" SDL2_image)
show_dependency_status("OPENAL" OPENAL)
show_dependency_status("PNG" PNG)
show_dependency_status("FREEALUT" FREEALUT)
show_dependency_status("TINYXML2" TINYXML2)

if(TARGET_OS STREQUAL "linux")
    show_dependency_status("GTK2" GTK2)
endif()

# -----------------------------------------------------
#   FOLDERS FOR INCLUDES
# -----------------------------------------------------

# Define folders where compiler should look for includes
set(ALL_INCLUDE_DIRS
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${LIBRARIES_DIR}
    ${LIBRARIES_DIR}/glad/include/
    ${EMULATOR_DIR}
    ${EDITCONTROLS_DIR}
    ${INFRASTRUCTURE_DIR}
    ${DEFINITIONS_DIR})
  
include_directories(${ALL_INCLUDE_DIRS})

if(TARGET_OS STREQUAL "linux")
    include_directories (${GTK2_INCLUDE_DIRS})
endif()

# -----------------------------------------------------
#   LINKED LIBRARIES FILES
# -----------------------------------------------------

# ADD EXTERNAL LIBRARIES
add_subdirectory(${LIBRARIES_DIR})

# Libraries to link with the emulator
set(EMULATOR_LIBS
    osdialog
    imgui
    ${TINYXML2_LIBRARY}
    ${PNG_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARY}
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${PNG_LIBRARY}
    glad
    ${OPENAL_LIBRARY}
    ${FREEALUT_LIBRARY}
    ${CMAKE_DL_LIBS})

# Libraries to link with the EditControls tool
set(EDITCONTROLS_LIBS
    osdialog
    imgui
    ${TINYXML2_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARY}
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${PNG_LIBRARY}
    glad
    ${CMAKE_DL_LIBS})

# -----------------------------------------------------
#   SOURCE FILES
# -----------------------------------------------------

# Source files to compile for the emulator
set(EMULATOR_SRC
    ${EMULATOR_DIR}/Globals.cpp
    ${EMULATOR_DIR}/GUI.cpp
    ${EMULATOR_DIR}/Languages.cpp
    ${EMULATOR_DIR}/Main.cpp
    ${EMULATOR_DIR}/Settings.cpp
    ${EMULATOR_DIR}/VirconBuses.cpp
    ${EMULATOR_DIR}/VirconCartridgeController.cpp
    ${EMULATOR_DIR}/VirconCPU.cpp
    ${EMULATOR_DIR}/VirconCPUProcessors.cpp
    ${EMULATOR_DIR}/VirconEmulator.cpp
    ${EMULATOR_DIR}/VirconGamepadController.cpp
    ${EMULATOR_DIR}/VirconGPU.cpp
    ${EMULATOR_DIR}/VirconGPUWriters.cpp
    ${EMULATOR_DIR}/VirconMemory.cpp
    ${EMULATOR_DIR}/VirconMemoryCardController.cpp
    ${EMULATOR_DIR}/VirconNullController.cpp
    ${EMULATOR_DIR}/VirconRNG.cpp
    ${EMULATOR_DIR}/VirconSPU.cpp
    ${EMULATOR_DIR}/VirconSPUThread.cpp
    ${EMULATOR_DIR}/VirconSPUWriters.cpp
    ${EMULATOR_DIR}/VirconTimer.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/LogStream.cpp
    ${INFRASTRUCTURE_DIR}/Matrix4D.cpp
    ${INFRASTRUCTURE_DIR}/OpenGL2DContext.cpp
    ${INFRASTRUCTURE_DIR}/StopWatch.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp
    ${INFRASTRUCTURE_DIR}/Texture.cpp
    ${DEFINITIONS_DIR}/VirconDefinitions.cpp
    ${DEFINITIONS_DIR}/VirconEnumerations.cpp
    ${DEFINITIONS_DIR}/VirconROMFormat.cpp)

# Source files to compile for the EditControls tool
set(EDITCONTROLS_SRC
    ${EDITCONTROLS_DIR}/Controls.cpp
    ${EDITCONTROLS_DIR}/Globals.cpp
    ${EDITCONTROLS_DIR}/GUI.cpp
    ${EDITCONTROLS_DIR}/Languages.cpp
    ${EDITCONTROLS_DIR}/Main.cpp
    ${INFRASTRUCTURE_DIR}/Definitions.cpp
    ${INFRASTRUCTURE_DIR}/FilePaths.cpp
    ${INFRASTRUCTURE_DIR}/LogStream.cpp
    ${INFRASTRUCTURE_DIR}/Matrix4D.cpp
    ${INFRASTRUCTURE_DIR}/OpenGL2DContext.cpp
    ${INFRASTRUCTURE_DIR}/StringFunctions.cpp
    ${INFRASTRUCTURE_DIR}/Texture.cpp)

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
    target_link_libraries(${EMULATOR_BINARY_NAME} ${GTK2_LIBRARIES})
    target_link_libraries(${EDITCONTROLS_BINARY_NAME} ${GTK2_LIBRARIES})
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

if(TARGET_OS STREQUAL "linux")
    # Install both binaries
    install(TARGETS ${EMULATOR_BINARY_NAME} ${EDITCONTROLS_BINARY_NAME}
        RUNTIME
        COMPONENT binaries
        DESTINATION Vircon32/Emulator)

    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION Vircon32/Emulator)
    # Install desktop entry
    install(FILES Resources/Linux/Vircon32.desktop DESTINATION /usr/share/applications )

    # Install icon
    install(FILES Resources/Linux/Vircon32.svg DESTINATION /usr/share/pixmaps )
    install(FILES Resources/Linux/Vircon32.svg DESTINATION /usr/share/icons/hicolor/scalable/apps/ )
	
elseif(TARGET_OS STREQUAL "windows")
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
        DESTINATION Emulator)

    # Copy the needed data files
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_DIR}/
        DESTINATION Emulator)
endif()
