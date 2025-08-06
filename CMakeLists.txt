cmake_minimum_required(VERSION 3.10)
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
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 示例程序
if(JADE_BUILD_EXAMPLES)
    # 使用共享库的示例
    set(SPDLOG_DIR "/usr/local/spdlog" CACHE PATH "location of libraries")
    if (NOT DEFINED SPDLOG_DIR OR SPDLOG_DIR STREQUAL "")
        message(FATAL_ERROR "please set SPDLOG_DIR with -DSPDLOG_DIR=/usr/local/spdlog")
    endif ()
    set(SPDLOG_INCLUDE_DIR "${SPDLOG_DIR}/include")
    set(SPDLOG_LIB_DIR "${SPDLOG_DIR}/lib")
    set(SPDLOG_LIBS spdlog)
    AUX_SOURCE_DIRECTORY(./src SRCS)
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}  ${SPDLOG_INCLUDE_DIR} )
    link_directories( ${SPDLOG_LIB_DIR} )
    add_executable(examples main.cpp ${SRCS}
            include/logger_impl.h
    )
    target_link_libraries(examples  ${SPDLOG_LIBS} )
    add_custom_command(TARGET examples POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SPDLOG_DIR}/bin/spdlog.dll ./Release/spdlog.dll
    )
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
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
    add_library(${APP_NAME} SHARED ${SRCS})
    target_include_directories(${APP_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
    target_compile_definitions(${APP_NAME} PRIVATE JADE_TOOLS_EXPORTS)
    # 导出目标供find_package使用
    install(TARGETS ${APP_NAME}
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin
            RUNTIME DESTINATION bin
    )
    install(DIRECTORY include/ DESTINATION include/jade_tools)
endif()
