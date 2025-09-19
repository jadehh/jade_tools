cmake_minimum_required(VERSION 3.10)
set(APP_NAME "jade_tools")
project(${APP_NAME} VERSION 1.0.0 LANGUAGES CXX)
if(CMAKE_GENERATOR_PLATFORM)
    # 如果定义了CMAKE_GENERATOR_PLATFORM，则直接使用
    if (CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        set(TARGET_ARCHITECTURE "x86")
    else ()
        set(TARGET_ARCHITECTURE ${CMAKE_GENERATOR_PLATFORM})
    endif ()
else()
    # 否则，根据指针大小判断
#    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#        set(TARGET_ARCHITECTURE "x64")
#    else()
#        set(TARGET_ARCHITECTURE "x86")
#    endif()
endif()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/.cmake/)
include(findSpdLog)
include(findOpenssl)
include(findCuda)
include(findBreakPad)
include(findSqlite3)
include(findOpencv)
include(findHaspAdapter)

option(JADE_BUILD_EXAMPLES "Build Examples" OFF)
option(BUILD_SHARED "Build Examples" OFF)



if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()


# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
AUX_SOURCE_DIRECTORY(./src SRCS)

if(NOT ${BUILD_OPENSSL_ON})
    list(REMOVE_ITEM SRCS ./src/crypto_utils.cpp)
endif ()

# 示例程序
if(JADE_BUILD_EXAMPLES)
    include(compileTest)
    if (BUILD_SHARED)
        include(findJadeTools)
        add_executable(examples_shared main.cpp )
        MESSAGE(${JADE_TOOLS_LIBS})
        target_link_libraries(examples_shared ${CUDA_LIBS} ${OPENSSL_LIBS} ${SQLITE3_LIBS} ${JADE_TOOLS_LIBS}  ${BREAKPAD_LIBS} ${HASP_ADAPTER_LIBS}  ${OPENCV_LIBS} jade_test)
        target_compile_definitions(examples_shared PRIVATE JADE_TOOLS)
    else ()
        add_executable(examples main.cpp ${SRCS})
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        target_link_libraries(examples ${CUDA_LIBS} ${OPENSSL_LIBS} ${SQLITE3_LIBS} ${BREAKPAD_LIBS} ${SPDLOG_LIBS} ${HASP_ADAPTER_LIBS}  ${OPENCV_LIBS}  jade_test )
        if(WIN32)
            target_compile_definitions(examples PRIVATE JADE_TOOLS_EXPORTS)
        endif()
        # --- 关键配置：强制为 Release 模式生成 PDB ---
        if(MSVC)
            # 只有当我们想要生成 PDB 且处于 Release 或 RelWithDebInfo 模式时才设置
            if(CMAKE_BUILD_TYPE STREQUAL "Release")
                message(STATUS "PDB generation enabled for MSVC compiler")
                # 为 Release 配置设置编译器标志 (/Zi 生成 PDB)
                set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
                # 为 Release 配置设置链接器标志 (/DEBUG 生成调试信息)
                set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
            endif ()
        else ()
        endif()
    endif ()
else ()
    # 输出目录设置
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    # 创建共享库 (DLL/SO)
    add_library(${APP_NAME} SHARED ${SRCS})
    if (MSVC)
        set_target_properties(${APP_NAME} PROPERTIES DEBUG_POSTFIX "d")
    endif ()
    target_compile_definitions(${APP_NAME} PRIVATE JADE_TOOLS_EXPORTS)
    target_link_libraries(${APP_NAME} ${CUDA_LIBS} ${OPENSSL_LIBS}  ${SQLITE3_LIBS} ${BREAKPAD_LIBS}  ${HASP_ADAPTER_LIBS}  ${OPENCV_LIBS})

    # 导出目标供find_package使用
    install(TARGETS ${APP_NAME}
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin
            RUNTIME DESTINATION bin
    )
    install(FILES  include/jade_tools.h DESTINATION include/jade_tools/)
endif()