cmake_minimum_required(VERSION 3.8)
project(RAKU LANGUAGES C VERSION 0.0.0)

option(RAKU_BUILD_TESTS  "Enable unit testing."         ON )
option(RAKU_BUILD_SHARED "Build RAKU's shared library." ON )
option(RAKU_BUILD_STATIC "Build RAKU's static library." OFF)

add_subdirectory(src)

# IF(RAKU_BUILD_TESTS)
#    add_subdirectory(tests)
# ENDIF()