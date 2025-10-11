# extractChangelog.cmake - 从CONTRIBUTING.md提取特定版本更新日志
# 用法：cmake -DREQUESTED_VERSION=v1.0.3 -DOUTPUT_FILE=RELEASE_NOTES.md -P extractChangelog.cmake

# 获取请求的版本号（默认为最新tag）
if(NOT REQUESTED_VERSION)
    execute_process(
            COMMAND git describe --tags --abbrev=0
            OUTPUT_VARIABLE REQUESTED_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# 设置默认输出文件名
if(NOT OUTPUT_FILE)
    set(OUTPUT_FILE "RELEASE_NOTES.md")
endif()

execute_process(
        COMMAND bash -c "basename $(git config --get remote.origin.url) .git"
        OUTPUT_VARIABLE REPO_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Repository name: ${REPO_NAME}")
message(STATUS "Output file: ${OUTPUT_FILE}")

# 验证文件存在
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md")
    message(FATAL_ERROR "CONTRIBUTING.md not found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# 读取Markdown内容
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/CONTRIBUTING.md" CONTRIB_MD)

# 清理HTML细节标签
string(REGEX REPLACE "<details[^>]*>.*</details>" "" CLEAN_CHANGELOG "${CONTRIB_MD}")
string(STRIP "${CLEAN_CHANGELOG}" CLEAN_CHANGELOG)
string(APPEND CLEAN_CHANGELOG "\n## 发布地址 \n [详细地址](https://github.com/jadehh/Release/releases/tag/${REPO_NAME}-${REQUESTED_VERSION})")
string(TIMESTAMP CURRENT_TIME "%Y-%m-%d %H:%M:%S")

set(CLEAN_CHANGELOG "# Jade Tools C++ 工具合集  \n ## 更新时间 \n  ${CURRENT_TIME} \n  ${CLEAN_CHANGELOG}")

# 写入文件
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT_FILE}" "${CLEAN_CHANGELOG}")

message(STATUS "Successfully extracted changelog for ${REQUESTED_VERSION}")
message(STATUS "Output file: ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT_FILE}")