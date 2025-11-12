cmake_minimum_required(VERSION 3.17)

set(STORM_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  StormLib
  GIT_REPOSITORY https://github.com/ladislav-zezula/StormLib.git
  GIT_TAG v9.25
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  PATCH_COMMAND python "${CMAKE_CURRENT_LIST_DIR}/../replace_all_names.py" && ${Patch_EXECUTABLE} --ignore-whitespace --verbose -p1 < "${CMAKE_CURRENT_LIST_DIR}/stormlib.patch" || cd .
)

FetchContent_MakeAvailable(StormLib)
