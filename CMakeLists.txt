cmake_minimum_required(VERSION 3.11)

if (DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(Simple_link_shortener VERSION 1.0.0)

add_executable(main src/main.cpp)

find_package(Crow CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE Crow::Crow)
target_link_libraries(main PRIVATE unofficial::sqlite3::sqlite3)