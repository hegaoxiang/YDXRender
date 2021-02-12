cmake_minimum_required(VERSION 3.8)

project("YDXRender")

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

aux_source_directory(${PROJECT_SOURCE_DIR}/3Party/imgui DIR_3Party)

add_library(${PROJECT_NAME} STATIC ${DIR_SRCS} ${DIR_3Party} )

target_include_directories(${PROJECT_NAME}
	PUBLIC 
	${PROJECT_SOURCE_DIR}/include 
	${PROJECT_SOURCE_DIR}/3Party)

add_executable(test_binary Test/Test.cpp)

target_link_libraries(test_binary PRIVATE YDXRender)