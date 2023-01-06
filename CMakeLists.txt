cmake_minimum_required(VERSION 3.16)

set(VERSION 1.7)
project(ModemInfo VERSION ${VERSION}
        DESCRIPTION "The ModemInfo was specially developed for the luci-app-modeminfo backend.\
                    Goal of the project is to provide a user-friendly interface\
                    for receiving information from 3G/LTE dongle.")

configure_file("${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
        )

include(FindPkgConfig)
find_library(cJSON cjson)
pkg_check_modules(QMILIB qmi-glib REQUIRED)
pkg_check_modules(GLIB glib-2.0 gio-2.0 REQUIRED)
pkg_check_modules(cJSON REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS} ${QMILIB_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})

set(GCC_COVERAGE_COMPILE_FLAGS "-Os")
set(CMAKE_CXX_FLAGS ${GCC_COVERAGE_COMPILE_FLAGS})

set(SOURCE_FILES src/main.cpp
        lib/ModemInfo/ModemInfo.cpp
        lib/ModemInfo/ModemInfo.h
        lib/ModemInfo/ModemInfoQmi.cpp
        lib/ModemInfo/ModemInfoQmi.h
        lib/ModemInfo/ModemInfoAt.cpp
        lib/ModemInfo/ModemInfoAt.h
        lib/Options/Options.cpp
        lib/Options/Options.h)

add_executable(${PROJECT_NAME} ${PROJECT_BINARY_DIR} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${QMILIB_LIBRARIES} ${cJSON})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        )

install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION /usr/bin/)