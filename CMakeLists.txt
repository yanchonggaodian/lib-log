cmake_minimum_required(VERSION 3.2.0)

include(GNUInstallDirs)

project(LOG C)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 5)
set(PROJECT_VERSION_PATCH 3)
set(LOG_VERSION_SO 1)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")


set(custom_compiler_flags)

include(CheckCCompilerFlag)
option(ENABLE_CUSTOM_COMPILER_FLAGS "Enables custom compiler flags for Clang and GCC" ON)
if (ENABLE_CUSTOM_COMPILER_FLAGS)
    list(APPEND custom_compiler_flags
        -std=c11
        -pedantic
        -Wall
        -Wextra
        -Werror
        -Wstrict-prototypes
        -Wwrite-strings
        -Wshadow
        -Winit-self
        -Wcast-align
        -Wformat=2
        -Wmissing-prototypes
        -Wstrict-overflow=2
        -Wcast-qual
        -Wundef
        -Wswitch-default
        -Wconversion
        -Wc++-compat
        -fstack-protector-strong
        -Wcomma
        -Wdouble-promotion
        -Wparentheses
        )
endif()

option(ENABLE_SANITIZERS "Enables AddressSanitizer and UndefinedBehaviorSanitizer." OFF)
if (ENABLE_SANITIZERS)
    list(APPEND custom_compiler_flags
        -fno-omit-frame-pointer
        -fsanitize=address
        -fsanitize=undefined
        -fsanitize=float-divide-by-zero
        -fsanitize=float-cast-overflow
        -fsanitize-address-use-after-scope
        -fsanitize=integer
        -01
        -fno-sanitize-recover
        )
endif()

# apply custom compiler flags
foreach(compiler_flag ${custom_compiler_flags})
    #remove problematic characters
    string(REGEX REPLACE "[^a-zA-Z0-9]" "" current_variable ${compiler_flag})

    CHECK_C_COMPILER_FLAG(${compiler_flag} "FLAG_SUPPORTED_${current_variable}")
    if (FLAG_SUPPORTED_${current_variable})
        list(APPEND supported_compiler_flags)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compiler_flag}")
    endif()
endforeach()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${supported_compiler_flags}")

#variables for pkg-config
message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}")
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(libdir "${CMAKE_INSTALL_LIBDIR}")
set(version "${PROJECT_VERSION}")
set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TARGET_EXPORT "Enable exporting of CMake targets. Disable when it causes problems!" ON)

#LOG
set(LOG_LIB log)

file(GLOB HEADERS log.h)
set(SOURCES log.c)
add_definitions(-D_XOPEN_SOURCE=500)
add_library("${LOG_LIB}" SHARED "${HEADERS}" "${SOURCES}")
find_library(PTHREAD_A libpthread.a)
add_library("${LOG_LIB}_static" STATIC "${HEADERS}" "${SOURCES}")
SET_TARGET_PROPERTIES("${LOG_LIB}_static" PROPERTIES OUTPUT_NAME "${LOG_LIB}") 
if (NOT WIN32)
	target_link_libraries("${LOG_LIB}" 
		pthread
		)
	target_link_libraries("${LOG_LIB}_static" LINK_INTERFACE_LIBRARIES
	     	pthread
		)
endif()


install(FILES log.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/log")
install(TARGETS "${LOG_LIB}" DESTINATION "${CMAKE_INSTALL_LIBDIR}" EXPORT "${LOG_LIB}")
if(ENABLE_TARGET_EXPORT)
    # export library information for CMake projects
    install(EXPORT "${LOG_LIB}" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/log")
endif()

set_target_properties("${LOG_LIB}"
    PROPERTIES
        SOVERSION "${LOG_VERSION_SO}"
        VERSION "${PROJECT_VERSION}")

# Install package config files
option(ENABLE_LOG_TEST "Enable building cJSON test" ON)
if(ENABLE_LOG_TEST)
    enable_testing()

    set(TEST_LOG log_test)
    add_executable("${TEST_LOG}" test.c)
    target_link_libraries("${TEST_LOG}" "${LOG_LIB}")

    add_test(NAME ${TEST_LOG} COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${TEST_LOG}")

    # Disable -fsanitize=float-divide-by-zero for cJSON_test
    if (FLAG_SUPPORTED_fsanitizefloatdividebyzero)
        if ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-sanitize=float-divide-by-zero")
        else()
		target_compile_options(${TEST_LOG} PRIVATE "-fno-sanitize=float-divide-by-zero")
        endif()
    endif()

    #"check" target that automatically builds everything and runs the tests
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${TEST_LOG})
endif()

