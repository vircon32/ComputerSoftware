##
## Honor tinyxml2_SHARED_LIBS to match install interface
##

if (DEFINED tinyxml2_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${tinyxml2_SHARED_LIBS}")
endif ()

##
## Main library build
##

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_library(tinyxml2 STATIC tinyxml2.cpp tinyxml2.h)
add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)

# Uncomment the following line to require C++11 (or greater) to use tinyxml2
# target_compile_features(tinyxml2 PUBLIC cxx_std_11)
target_include_directories(tinyxml2 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_compile_definitions(
    tinyxml2
    PUBLIC $<$<CONFIG:Debug>:TINYXML2_DEBUG>
    INTERFACE $<$<BOOL:${BUILD_SHARED_LIBS}>:TINYXML2_IMPORT>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    PUBLIC _FILE_OFFSET_BITS=64
)

set_target_properties(
    tinyxml2
    PROPERTIES
    DEFINE_SYMBOL "TINYXML2_EXPORT"
    VERSION "${tinyxml2_VERSION}"
    SOVERSION "${tinyxml2_VERSION_MAJOR}"
)
