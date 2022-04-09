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

# Source files to compile dialog library osdialog;
# These are dependent on the backend GUI system
# that osdialog is targeting (win/gtk2/gtk3/zenity)
add_library(osdialog STATIC
    osdialog/osdialog.c
    osdialog/osdialog_${OSDIALOG_TARGET}.c)
