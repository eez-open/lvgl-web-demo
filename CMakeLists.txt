cmake_minimum_required(VERSION 3.12)
project (lvgl-web-demo)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2 -s USE_SDL=2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s INITIAL_MEMORY=83886080 -sLLD_REPORT_UNDEFINED -s USE_SDL=2")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(lvgl)
add_subdirectory(lv_drivers)
file(GLOB MY_SOURCES "./*.c")
set(SOURCES ${MY_SOURCES})

# Comment out following statements to build project without eez flow support.
# Also, build lvgl-demo-no-flow.eez-project with EEZ Studio.
add_definitions(-DHAS_FLOW_SUPPORT=1)
set (HAS_FLOW_SUPPORT 1)

file(GLOB_RECURSE my_src
    ./src/*.cpp
    ./src/*.c
)
list(APPEND SOURCES ${my_src})

if (${HAS_FLOW_SUPPORT})
    ################################################################################
    # EEZ Framework
    add_definitions(-DEEZ_PLATFORM_SIMULATOR)
    add_definitions(-DEEZ_FOR_LVGL)
    add_definitions(-DEEZ_OPTION_GUI=0)

    include_directories(
        ./
        ./eez/libs/agg
        ./eez/platform/simulator
        ./src/conf
    )

    file(GLOB_RECURSE src_eez
        ./eez/*.cpp
        ./eez/*.c
    )

    list(APPEND SOURCES ${src_eez})

    ################################################################################
endif(${HAS_FLOW_SUPPORT})

add_executable(index ${SOURCES} ${INCLUDES})

if(NOT LVGL_CHOSEN_DEMO)
    set(LVGL_CHOSEN_DEMO lv_demo_widgets)
endif()
set_source_files_properties(main.c PROPERTIES COMPILE_FLAGS -DCHOSEN_DEMO=${LVGL_CHOSEN_DEMO})

set(CMAKE_EXECUTABLE_SUFFIX ".html")
target_link_libraries(index
    lvgl
    lv_drivers
)
set_target_properties(index PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/lvgl_shell.html -s SINGLE_FILE=1")
