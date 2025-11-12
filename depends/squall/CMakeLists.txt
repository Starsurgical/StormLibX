cmake_minimum_required(VERSION 3.17)
include(FetchContent)

set(WHOA_STORM_FLAVOR "SC1" CACHE STRING "" FORCE)

FetchContent_Declare(
  Squall
  GIT_REPOSITORY https://github.com/Starsurgical/squall.git
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  CMAKE_ARGS -DWHOA_STORM_FLAVOR=SC1
)

FetchContent_MakeAvailable(Squall)

set(squall_SOURCE_DIR ${squall_SOURCE_DIR} PARENT_SCOPE)
