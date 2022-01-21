cmake_minimum_required(VERSION 3.11)

#设置项目名
project(five_fifty VERSION 1.0.0 LANGUAGES C CXX)

#设置编译器标准
set(CMAKE_C_STANDARD 99)

#CMake版本小于3.7.0时启用CMAKE_INCLUDE_CURRENT_DIR目录
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

#添加宏
IF (UNIX)
    SET(CURSES_NEED_NCURSES TRUE)
    SET(define_lfs_macros TRUE)
    IF (define_lfs_macros)
        add_definitions(-D__USE_FILE_OFFSET64)
        add_definitions(-D__USE_LARGEFILE64)
        add_definitions(-D_LARGEFILE64_SOURCE)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    ENDIF ()
ENDIF ()


#查找依赖的软件包
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(PostgreSQL REQUIRED)
IF (UNIX)
     find_package(Curses REQUIRED)
    IF (CURSES_FOUND AND CURSES_HAVE_NCURSES_H)
        include_directories(${CURSES_INCLUDE_DIR})
    ENDIF ()
    find_package(Threads REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JANSSON REQUIRED jansson)
    find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/include NO_DEFAULT_PATH)
    find_library(KMBLACK_LIB libkmblack.so ${KMBLACK_DIR}/lib)

     find_path(UDP_SERVER_INCLUDE NAMES KcPlugins.h PATHS ${UDP_SERVER_DIR} NO_DEFAULT_PATH)
ELSE ()
    find_package(pthreads REQUIRED)
    find_package(iconv REQUIRED)
    find_package(Intl REQUIRED)
    find_package(JANSSON CONFIG REQUIRED)
    find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/kmblack NO_DEFAULT_PATH)
    find_library(KMBLACK_LIB kmblack.lib "${KMBLACK_DIR}/x64/Release")
ENDIF ()

#添加所有的源文件
SET(SRC
        KcProcessData.c
)

#导入头文件目录
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${KMBLACK_INCLUDE} ${UDP_SERVER_INCLUDE})

#生成动态链接库
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC})

#为target添加需要链接的共享库
IF (WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} jansson::jansson ws2_32 DbgHelp ${Intl_LIBRARIES} ${PThreads4W_LIBRARY_RELEASE} ${KMBLACK_LIB})
ELSE ()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} -lpthread -lm -ldl -lcurses jansson ${Intl_LIBRARIES}  ${PostgreSQL_LIBRARY_RELEASE}  ${KMBLACK_LIB})
ENDIF ()

# Installation
SET(install_targets ${install_targets} ${PROJECT_NAME})
INSTALL(TARGETS ${install_targets}
        #可执行二进制文件
        RUNTIME DESTINATION bin
        #动态库
        LIBRARY DESTINATION .
        #静态库
        ARCHIVE DESTINATION .
        )



STRING(TOUPPER "${CMAKE_BUILD_TYPE}" buildtype)
MESSAGE(STATUS "")
MESSAGE(STATUS "")
MESSAGE(STATUS "kmblack configuration summary:")
MESSAGE(STATUS "")

MESSAGE(STATUS "  Build type ...................... : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "  Install .pdb (if available)...... : ${INSTALL_PDB}")
MESSAGE(STATUS "  Install prefix .................. : ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "  C compiler ...................... : ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "  Minimum Windows version ......... : ${MIN_WINDOWS_VER}")
