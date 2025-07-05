cmake_minimum_required(VERSION 3.16)

# 项目名称
project(maque)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 自动查找src目录下的所有源文件
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/iniparser/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/iniparser/src/*.h"
)
# 指定包含目录
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/iniparser/src")

# 创建可执行文件（明确指定maque.c为主文件）
add_executable(maque 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/maque.c"
    ${SOURCES}
)

# 安装目标（可选）
install(TARGETS maque DESTINATION bin)



# # 包含的头文件目录
# # include_directories(./include)
# include_directories(${PROJECT_SOURCE_DIR}/src)
# set(SRC_DIR ./src)
# # 包含的源文件目录
# file(GLOB_RECURSE SRC_FILES ${SRC_DIR}/*.c)

# # 指定生产链接库
# add_library(maque ${SRC_FILES})

# # 设置变量
# set(LIBRARIES maque)
# set(OBJECT maque)

# # 生成可执行文件
# add_executable(${OBJECT} maque.c)

# # 为可执行文件链接目标库
# target_link_libraries(${OBJECT} ${LIBRARIES} m)