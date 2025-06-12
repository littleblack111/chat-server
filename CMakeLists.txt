cmake_minimum_required(VERSION 4.0.2)

project(
  chat-server
  VERSION 1.0.0
  DESCRIPTION "chat server via socket"
  LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  message(STATUS "Configuring chat-server in Debug with CMake")
  add_compile_definitions(DEBUG)
else()
  message(STATUS "Configuring chat-server in Release with CMake")
endif()

message(STATUS "Checking dependencies...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(FTXUI REQUIRED IMPORTED_TARGET ftxui)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_26)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
                    -Wno-unused-parameter)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  message(STATUS "Setting debug flags")
  add_compile_options(-g3 -O0 -fno-omit-frame-pointer)
  add_compile_options(-fsanitize=address,undefined)
  target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
else()
  add_compile_options(-O3 -flto -DNDEBUG)
  target_link_options(${PROJECT_NAME} PRIVATE -flto -Wl,--gc-sections)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::FTXUI)
target_include_directories(${PROJECT_NAME} PRIVATE src)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
