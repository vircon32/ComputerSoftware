# source files to compile graphic library glad
add_library(glad STATIC
    glad/src/glad.c)

# source files to compile GUI library imgui
add_library(imgui STATIC
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl2.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp)

# select a specific GL loader or it will attempt to use GLEW
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Source files to compile dialog library osdialog;
# These are dependent on the backend GUI system
# that osdialog is targeting (win/mac/gtk2/gtk3/zenity)
if(OSDIALOG_TARGET STREQUAL "mac")
    add_library(osdialog STATIC
        osdialog/osdialog.c
        osdialog/osdialog_mac.m)
else()
    add_library(osdialog STATIC
        osdialog/osdialog.c
        osdialog/osdialog_${OSDIALOG_TARGET}.c)
endif()

# source files to compile XML library tinyxml2
# (ignore the original CMake script)
add_library(tinyxml2 STATIC
    tinyxml2/tinyxml2.cpp)

