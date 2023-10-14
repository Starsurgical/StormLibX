project(StormLibX)
cmake_minimum_required(VERSION 3.12)

############################################################
# Options
############################################################
set(THIS_LIBRARY_NAME StormLibX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Compile shared libraries" OFF)
option()

############################################################
# Loading StormLib
############################################################
set(STORM_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "" FORCE)

set(LIBRARY_NAME StormLib PARENT_SCOPE)
add_subdirectory(libs/StormLib)

############################################################
# Loading squall
############################################################
#add_subdirectory(libs/squall)


############################################################
# Project Config
############################################################
set(SRC_FILES
          src/SBig.cpp
          src/SBlt.cpp
          src/SBmp.cpp
          src/SCmd.cpp
          src/SCode.cpp
          src/SComp.cpp
          src/SDlg.cpp
          src/SDraw.cpp
          src/SErr.cpp
          src/SEvt.cpp
          src/SFile.cpp
          src/SGdi.cpp
          src/SLog.cpp
          src/SMem.cpp
          src/SMsg.cpp
          src/SNet.cpp
          src/SReg.cpp
          src/SRgn.cpp
          src/SSig.cpp
          src/SStr.cpp
          src/STrans.cpp
          src/SUni.cpp
          src/SVid.cpp
          src/Storm.cpp
)

file(GLOB_RECURSE INTERFACE_FILES include/*.h)

if(MSVC)
    # This file is used to create a DLL on windows
    # Use BUILD_SHARED_LIBS to create StormLib.dll
    set(STORM_DEF_FILES
           src/DllMain.def
    )
endif()

add_library(${THIS_LIBRARY_NAME} ${LIB_TYPE} ${SRC_FILES} ${STORM_DEF_FILES} ${INTERFACE_FILES})
if(WIN32)
    set_target_properties(${THIS_LIBRARY_NAME} PROPERTIES OUTPUT_NAME "StormLibX")
endif()

target_link_libraries(${THIS_LIBRARY_NAME} PRIVATE StormLib)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SRC_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "Header Files" FILES ${INTERFACE_FILES})

target_include_directories(${THIS_LIBRARY_NAME} PUBLIC include/)

