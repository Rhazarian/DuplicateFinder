cmake_minimum_required(VERSION 3.12)
project(DuplicateFinder)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")

set(CMAKE_PREFIX_PATH "/usr/local/Cellar/qt/5.11.2/lib/cmake")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Quick REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Quick_INCLUDE_DIRS})

add_executable(DuplicateFinder main.cpp
        main_window.cpp main_window.h main_window.ui
        duplicate_finder.h duplicate_finder.cpp
        popup_window.cpp popup_window.h popup_window.ui
        error_popup_window.cpp error_popup_window.h error_popup_window.ui
        delete_popup_window.cpp delete_popup_window.h delete_popup_window.ui
        find_duplicates.h find_duplicates.cpp)

target_link_libraries(DuplicateFinder Qt5::Core)
target_link_libraries(DuplicateFinder Qt5::Widgets)
target_link_libraries(DuplicateFinder Qt5::Quick)
target_link_libraries(DuplicateFinder stdc++fs)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIR})

target_link_libraries(DuplicateFinder ${Boost_LIBRARIES})
