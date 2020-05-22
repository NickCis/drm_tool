project(drm_tool)
cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(LibDrm REQUIRED)

SET(DRMTOOL_SOURCES src/drm_tool.c)

add_executable(${PROJECT_NAME} ${DRMTOOL_SOURCES})                    
target_link_libraries(${PROJECT_NAME} LIBDRM::LIBDRM)
target_compile_definitions(${PROJECT_NAME} PRIVATE -D_FILE_OFFSET_BITS=64)
set_target_properties (${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_g")