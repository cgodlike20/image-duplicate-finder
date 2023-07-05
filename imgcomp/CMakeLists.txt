cmake_minimum_required(VERSION 3.0.0)
project(imgcomp VERSION 0.1.0)

#include(CTest)
#enable_testing()
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_DEFAULT_BUILD_TYPE Release)

add_executable(imgcomp src/main.cpp src/imgcmp.cpp)
target_compile_features(imgcomp PUBLIC cxx_std_17)

find_package(OpenCV REQUIRED)
find_package(libpqxx REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(C:/Libpqxx/libpqxx/include)

target_link_libraries(imgcomp ${OpenCV_LIBS})
target_link_libraries(imgcomp libpqxx::pqxx)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)