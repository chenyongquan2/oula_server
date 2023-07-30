cmake_minimum_required(VERSION 3.0.0)
project(oula_server VERSION 0.1.0 LANGUAGES C CXX)

#支持c++20标准
set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

#file(GLOB SOURCES "*.cpp" "*.h")
#add_executable(${PROJECT_NAME} ${SOURCES})

add_executable(${PROJECT_NAME})
# 递归获取所有 .cpp 和 .h 文件  
file(GLOB_RECURSE SOURCE_FILES  
    "${CMAKE_SOURCE_DIR}/*.cpp"  
    "${CMAKE_SOURCE_DIR}/*.h"  
)
list(FILTER SOURCE_FILES EXCLUDE REGEX "build/")
# 将文件添加到可执行文件中  
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})  


#input your vcpkg path
set(CMAKE_PREFIX_PATH "/home/cyq/vcpkg/packages")

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} pthread CURL::libcurl cryptopp::cryptopp fmt::fmt)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
