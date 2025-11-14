cmake_minimum_required(VERSION 3.17)
project(StormLibX)

include(FetchContent)
find_package(Patch REQUIRED)

############################################################
# Options
############################################################
set(THIS_LIBRARY_NAME StormLibX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Compile shared libraries" OFF)

############################################################
# Dependencies
############################################################
add_subdirectory("depends")

if(DEFINED squall_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH
        "${squall_SOURCE_DIR}/lib/system/cmake"
        "${squall_SOURCE_DIR}/vendor/cmake-modules"
    )

    include(system) # defines WHOA_SYSTEM_WIN
else()
    message(FATAL_ERROR "Squall source directory not defined. Cannot include system.cmake")
endif()

############################################################
# Project Config
############################################################
set(SRC_FILES
          src/SBlt.cpp
          src/SBmp.cpp
          src/SCmd.cpp
          src/SCode.cpp
          src/SComp.cpp
          src/SDlg.cpp
          src/SDraw.cpp
          src/SErr.cpp
          src/SFile.cpp
          src/SGdi.cpp
          src/SLog.cpp
          src/SMsg.cpp
          src/SNet.cpp
          src/SReg.cpp
          src/SVid.cpp
          src/Storm.cpp
          src/DllMain.cpp
)

file(GLOB_RECURSE INTERFACE_FILES include/*.h)

if(MSVC)
    # This file is used to create a DLL on windows
    # Use BUILD_SHARED_LIBS to create StormLib.dll
    set(STORM_DEF_FILES src/DllMain.def)
endif()

add_library(${THIS_LIBRARY_NAME} ${LIB_TYPE} ${SRC_FILES} ${STORM_DEF_FILES} ${INTERFACE_FILES})
if(WIN32)
    set_target_properties(${THIS_LIBRARY_NAME} PROPERTIES OUTPUT_NAME "StormLibX")
endif()

target_link_libraries(${THIS_LIBRARY_NAME} PUBLIC stormlib PUBLIC storm PRIVATE SDL2::SDL2-static PRIVATE tomlplusplus::tomlplusplus PRIVATE SDL2_image)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "Header Files" FILES ${INTERFACE_FILES})

target_include_directories(${THIS_LIBRARY_NAME} PUBLIC include/)
target_compile_definitions(StormLibX PRIVATE WHOA_STORM_FLAVOR=SC1)
