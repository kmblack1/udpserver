cmake_minimum_required(VERSION 3.11)

#设置项目名
project(part01 VERSION 1.0.0 LANGUAGES C CXX)

#设置编译器标准
set(CMAKE_C_STANDARD 99)

#CMake版本小于3.7.0时启用CMAKE_INCLUDE_CURRENT_DIR目录
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

#判断操作系统环境
IF (UNIX)
ELSE ()
ENDIF ()

#查找依赖的软件包


#添加所有的源文件
SET(SRC
        main.c
)

#生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME}
        ${SRC}
)

#为target添加需要链接的共享库
