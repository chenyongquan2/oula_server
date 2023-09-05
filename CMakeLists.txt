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

# 打印SOURCE_FILES
message("SOURCE_FILES: ${SOURCE_FILES}")

# 下载并构建spdlog库  
include(ExternalProject)  
ExternalProject_Add(spdlog  
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/spdlog  
    GIT_REPOSITORY https://github.com/gabime/spdlog.git  
    GIT_TAG master  
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/spdlog  
)  
  
# 添加spdlog库的头文件搜索路径  
include_directories(${CMAKE_CURRENT_BINARY_DIR}/spdlog/include)  
  
# 添加spdlog库的链接路径  
link_directories(${CMAKE_CURRENT_BINARY_DIR}/spdlog/lib)  

#input your vcpkg path
set(CMAKE_PREFIX_PATH "/home/cyq/vcpkg/packages")

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)

#本来想用vcpkg来安装使用spdlog的，但是不知道为啥最新的包只有v.12.0远远落后于最新的包。
#find_package(spdlog CONFIG REQUIRED)
# spdlog::spdlog

target_link_libraries(${PROJECT_NAME} pthread CURL::libcurl cryptopp::cryptopp fmt::fmt)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
