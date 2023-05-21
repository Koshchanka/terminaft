cmake_minimum_required(VERSION 3.11)

project(TUI
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-fsanitize=address -O2")

include_directories(".")

add_library(braille
    braille/dots.h
    braille/dots.cpp
)

add_library(math
    math/3d.h
    math/3d.cpp
    math/common.h
)

add_library(tui
    tui/char.h
    tui/color.h
    tui/utf8.h
    tui/utils.cpp
    tui/utils.h
    tui/view_port.cpp
    tui/view_port.h
)

add_library(input
    input/input.h
    input/input.cpp
    input/event.h
)

add_executable(main
    main.cpp
)
target_link_libraries(
    main PUBLIC tui braille math input
)

