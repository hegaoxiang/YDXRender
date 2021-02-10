cmake_minimum_required(VERSION 3.8)

project("YDXRender")

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(${PROJECT_NAME} STATIC ${DIR_SRCS} )

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)