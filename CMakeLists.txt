cmake_minimum_required(VERSION 3.11)
project(BIN_VIEWER CXX)

set(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

# The project needs the Qt5_DIR System Environment Variable 
# Example: "C:\QtBase\Qt5.9.8\5.9.8\msvc2017_64\lib\cmake\Qt5"

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Instruct CMake to include project Qt resources as needed
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

include_directories(BIN_VIEWER
        header
        glres/include)

if(MSVC)
        # Similar to -Wall -Wextra
        add_compile_options(/W3 /WX)

        # Type conversion warnings
        add_compile_options("/wd4244") # type conversion
        add_compile_options("/wd4305") # type truncation
        add_compile_options("/wd4267") # type conversion, loss of data

elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-Wall -Wextra -Wno-sign-compare)
endif(MSVC)

# add_compile_options(-Wall -W4)

add_executable(BIN_VIEWER
        source/bayer.cpp
        source/dot_plot.cpp
        source/binary_viewer.cpp
        source/plot_view.cpp
        source/hilbert.cpp
        source/histogram_calc.cpp
        source/overall_view.cpp
        source/histogram_2d_view.cpp
        source/image_view.cpp
        source/main.cpp
        source/main_app.cpp
        source/version.cpp
        source/histogram_3d_view.cpp
        header/bayer.h
        header/binary_viewer.h
        header/dot_plot.h
        header/plot_view.h
        header/hilbert.h
        header/histogram_calc.h
        header/overall_view.h
        header/histogram_2d_view.h
        header/image_view.h
        header/main_app.h
        header/version.h
        header/histogram_3d_view.h
        qdark/style.qrc
        glres/include/glut.h)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL)
target_link_libraries(BIN_VIEWER 
        Qt5::Core 
        Qt5::Widgets 
        Qt5::Gui 
        Qt5::OpenGL)

target_link_libraries(BIN_VIEWER
        ../glres/library/GL 
        ../glres/library/GLU
        ../glres/library/glut
        ../glres/library/glut32)

add_custom_command(TARGET BIN_VIEWER POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/qtres/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:BIN_VIEWER>)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "qt5binvis")