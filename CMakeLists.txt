cmake_minimum_required(VERSION 3.11)

if (DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(Simple_link_shortener VERSION 1.0.0)

find_package(Crow CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

add_library(database STATIC src/database.cpp)
target_include_directories(database PUBLIC include)
target_link_libraries(database PRIVATE unofficial::sqlite3::sqlite3)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE database Crow::Crow)
target_link_libraries(main PRIVATE cpr::cpr)