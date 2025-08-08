cmake_minimum_required(VERSION 3.10)
include(cmake/findCuda.cmake)
include(cmake/findSpdLog.cmake)

set(APP_NAME "jade_tools")
project(${APP_NAME} VERSION 1.0.0 LANGUAGES CXX)
option(JADE_BUILD_EXAMPLES "Build Examples" OFF)
if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}  ${SPDLOG_INCLUDE_DIR}  ${CUDA_INCLUDE_DIR})
link_directories(${CUDA_LIB_DIR})
# 示例程序
if(JADE_BUILD_EXAMPLES)
    AUX_SOURCE_DIRECTORY(./src SRCS)
    add_executable(examples main.cpp ${SRCS} )
    target_link_libraries(examples ${CUDA_LIBS})
    if(WIN32)
        target_compile_definitions(examples PRIVATE JADE_TOOLS_EXPORTS)
    endif()
else ()
    # 输出目录设置
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    AUX_SOURCE_DIRECTORY(./src SRCS)
    # 创建共享库 (DLL/SO)
    add_library(${APP_NAME} SHARED ${SRCS})
    target_compile_definitions(${APP_NAME} PRIVATE JADE_TOOLS_EXPORTS)
    target_link_libraries(${APP_NAME} ${CUDA_LIBS})

    # 导出目标供find_package使用
    install(TARGETS ${APP_NAME}
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin
            RUNTIME DESTINATION bin
    )
    install(DIRECTORY include/ DESTINATION include/jade_tools)
endif()