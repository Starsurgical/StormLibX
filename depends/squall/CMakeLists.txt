cmake_minimum_required(VERSION 3.12)

FetchContent_Declare(
  Squall
  GIT_REPOSITORY https://github.com/Starsurgical/squall.git
  GIT_SHALLOW 1
  GIT_PROGRESS 1
  PATCH_COMMAND python "${CMAKE_CURRENT_LIST_DIR}/../replace_all_names.py"
)

FetchContent_MakeAvailable(Squall)
