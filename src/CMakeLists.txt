cmake_minimum_required(VERSION 3.8)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(
    SOURCES
        string.c
        core/log.c
        core/status.c
        core/memory.c
        json/json_values.h
        json/json_values.c
)

set(
    DEBUG_SOURCES
        debug/assert.c
)

set(RAKU_COMPILE_OPTIONS)
set(RAKU_COMPILE_DEBUG_OPTIONS)

IF(MSVC)
    list(
        APPEND RAKU_COMPILE_OPTIONS
            /O2
    )
    list(
        APPEND RAKU_COMPILE_DEBUG_OPTIONS
            /Ob1 /DEBUG:FULL
    )
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(
        APPEND RAKU_COMPILE_OPTIONS
            -O3
    )
    list(
        APPEND RAKU_COMPILE_DEBUG_OPTIONS
            -O0 -g -Wall
    )
ELSEIF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    list(
        APPEND RAKU_COMPILE_OPTIONS
            -O3
    )
    list(
        APPEND RAKU_COMPILE_DEBUG_OPTIONS
            -O0 -g -Wall
    )
ELSE()
    message(FATAL_ERROR "Compiler not supported!")
ENDIF()

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

IF (CMAKE_GENERATOR MATCHES "Visual Studio")
    IF (RAKU_BUILD_SHARED OR NOT RAKU_BUILD_STATIC)
        add_library(${PROJECT_NAME} SHARED ${SOURCES} $<IF:$<CONFIG:Debug>,${DEBUG_SOURCES},>)
        target_compile_definitions(
            ${PROJECT_NAME} PRIVATE
                _CRT_SECURE_NO_WARNINGS
                RAKU_BUILD
                $<IF:$<CONFIG:Debug>,RAKU_DEBUG,>
        )
        set_target_properties(
            ${PROJECT_NAME} PROPERTIES
                C_STANDARD_REQUIRED                 11
                COMPILE_OPTIONS                     "$<IF:$<CONFIG:Debug>,${RAKU_COMPILE_DEBUG_OPTIONS},${RAKU_COMPILE_OPTIONS}>"

                OUTPUT_NAME_DEBUG                   "${PROJECT_NAME}-d"
                ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/bin"

                OUTPUT_NAME_RELEASE                 "${PROJECT_NAME}"
                ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/bin"
        )
    ENDIF()
    IF (RAKU_BUILD_STATIC)
        add_library(${PROJECT_NAME}-s STATIC ${SOURCES} $<IF:$<CONFIG:Debug>,${DEBUG_SOURCES},>)
        target_compile_definitions(
            ${PROJECT_NAME}-s PRIVATE
                _CRT_SECURE_NO_WARNINGS
                RAKU_BUILD
                RAKU_STATIC
                $<IF:$<CONFIG:Debug>,RAKU_DEBUG,>
        )
        set_target_properties(
            ${PROJECT_NAME}-s PROPERTIES
                C_STANDARD_REQUIRED                 11
                COMPILE_OPTIONS                     "$<IF:$<CONFIG:Debug>,${RAKU_COMPILE_DEBUG_OPTIONS},${RAKU_COMPILE_OPTIONS}>"

                OUTPUT_NAME_DEBUG                   "${PROJECT_NAME}-sd"
                ARCHIVE_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${PROJECT_SOURCE_DIR}/bin"

                OUTPUT_NAME_RELEASE                 "${PROJECT_NAME}-s"
                ARCHIVE_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${PROJECT_SOURCE_DIR}/bin"
        )
    ENDIF()
ELSE()
    IF (RAKU_BUILD_SHARED OR NOT RAKU_BUILD_STATIC)
        add_library(${PROJECT_NAME} SHARED ${SOURCES})
        target_compile_definitions(
            ${PROJECT_NAME} PRIVATE
                RAKU_BUILD
        )
        target_compile_options(
            ${PROJECT_NAME} PRIVATE
                ${RAKU_COMPILE_OPTIONS}
        )

        add_library(${PROJECT_NAME}-d SHARED ${SOURCES} ${DEBUG_SOURCES})
        target_compile_definitions(
            ${PROJECT_NAME}-d PRIVATE
                RAKU_BUILD
                RAKU_DEBUG
        )
        target_compile_options(
            ${PROJECT_NAME}-d PRIVATE
                ${RAKU_COMPILE_DEBUG_OPTIONS}
        )

        set_target_properties(
            ${PROJECT_NAME} ${PROJECT_NAME}-d
                PROPERTIES
                    C_STANDARD_REQUIRED         11
                    ARCHIVE_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/lib"
                    LIBRARY_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/lib"
                    RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/bin"
        )
    ENDIF()

    IF(RAKU_BUILD_STATIC)
        add_library(${PROJECT_NAME}-s STATIC ${SOURCES})
        target_compile_definitions(
            ${PROJECT_NAME}-s PRIVATE
                RAKU_STATIC
                RAKU_BUILD
        )
        target_compile_options(
            ${PROJECT_NAME}-s PRIVATE
                ${RAKU_COMPILE_OPTIONS}
        )

        add_library(${PROJECT_NAME}-sd STATIC ${SOURCES} ${DEBUG_SOURCES})
        target_compile_definitions(
            ${PROJECT_NAME}-sd PRIVATE
                RAKU_STATIC
                RAKU_BUILD
                RAKU_DEBUG
        )
        target_compile_options(
            ${PROJECT_NAME}-sd PRIVATE
                ${RAKU_COMPILE_DEBUG_OPTIONS}
        )

        set_target_properties(
            ${PROJECT_NAME}-s ${PROJECT_NAME}-sd
                PROPERTIES
                    C_STANDARD_REQUIRED         11
                    ARCHIVE_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/lib"
                    LIBRARY_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/lib"
                    RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/bin"
        )
    ENDIF()
ENDIF()