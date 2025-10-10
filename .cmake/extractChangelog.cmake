# extractChangelog.cmake - 从CONTRIBUTING.md提取特定版本更新日志
# 用法：cmake -DREQUESTED_VERSION=v1.0.3 -P extractChangelog.cmake

# 获取请求的版本号（默认为最新tag）
if(NOT REQUESTED_VERSION)
    execute_process(
            COMMAND git describe --tags --abbrev=0
            OUTPUT_VARIABLE REQUESTED_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# 验证文件存在
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md")
    message(FATAL_ERROR "CONTRIBUTING.md not found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# 读取Markdown内容
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md" CONTRIB_MD)

# 清理HTML细节标签
string(REGEX REPLACE "<details[^>]*>.*</details>" "" CLEAN_CHANGELOG "${CONTRIB_MD}")
string(STRIP "${CLEAN_CHANGELOG}" CLEAN_CHANGELOG)

# 输出文件路径
set(OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG_TAG.md")

# 写入文件
file(WRITE "${OUTPUT_FILE}" "${CLEAN_CHANGELOG}")

message(STATUS "Successfully extracted changelog for ${REQUESTED_VERSION}")
message(STATUS "Output file: ${OUTPUT_FILE}")