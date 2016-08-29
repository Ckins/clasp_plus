file(GLOB LIBCLASP_SRC src/*.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_library(libclasp STATIC ${LIBCLASP_SRC} clasp/sysu_program.h src/sysu_program.cpp clasp/sysu_dependency_graph.h src/sysu_dependency_graph.cpp)
