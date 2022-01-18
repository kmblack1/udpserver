cmake_minimum_required(VERSION 3.11)

#设置项目名
project(udpserver VERSION 1.0.0 LANGUAGES C CXX)

#设置编译器标准
set(CMAKE_C_STANDARD 99)

#CMake版本小于3.7.0时启用CMAKE_INCLUDE_CURRENT_DIR目录
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()



#查找依赖的软件包
find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/include NO_DEFAULT_PATH)
#判断操作系统环境
IF (UNIX)
    find_library(KMBLACK_LIB libkmblack.so ${KMBLACK_DIR}/lib)
ELSE ()
    find_package(pthreads REQUIRED)
    find_package(iconv REQUIRED)
    find_package(Intl REQUIRED)
    find_library(KMBLACK_LIB kmblack.lib ${KMBLACK_DIR}/lib)
ENDIF ()

#添加所有的源文件
SET(SRC
        main.c
)

#导入头文件目录
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${KMBLACK_INCLUDE})

#生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME}
        ${SRC}
)

#为target添加需要链接的共享库
IF (WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Intl_LIBRARIES} ${PThreads4W_LIBRARY_RELEASE} ${KMBLACK_LIB})
ELSE ()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} -lpthread -lm -ldl -lcurses ${Intl_LIBRARIES}  ${KMBLACK_LIB})
ENDIF ()
