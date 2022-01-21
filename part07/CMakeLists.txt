cmake_minimum_required(VERSION 3.11)

#设置项目名
project(part07 VERSION 1.0.0 LANGUAGES C CXX)

#设置编译器标准
set(CMAKE_C_STANDARD 99)

#CMake版本小于3.7.0时启用CMAKE_INCLUDE_CURRENT_DIR目录
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

#定义WIN32_LEAN_AND_MEAN宏
IF (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
ENDIF ()

#查找依赖的软件包
IF (UNIX)
    find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/include NO_DEFAULT_PATH)
    find_library(KMBLACK_LIB libkmblack.so ${KMBLACK_DIR}/lib)
ELSE ()
    find_package(Intl REQUIRED)
    find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/kmblack NO_DEFAULT_PATH)
    find_library(KMBLACK_LIB kmblack.lib "${KMBLACK_DIR}/x64/Debug")
ENDIF()

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
