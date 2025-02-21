
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10.0)

add_library(badgelib
    src/arrays.c
    src/badge_err.c
    src/badge_strings.c
    src/list.c
    src/refcount.c
)
target_include_directories(badgelib PUBLIC include)
