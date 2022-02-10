cmake_minimum_required(VERSION 3.11)

#设置项目名
project(kmblack VERSION 1.0.0 LANGUAGES C CXX)

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
IF (WIN32)
    find_package(JANSSON CONFIG REQUIRED)
    find_package(iconv REQUIRED)
    find_package(Intl REQUIRED)
    find_package(pthreads REQUIRED)
ELSE ()
    find_package(PkgConfig)
    pkg_check_modules(OSSP-UUID REQUIRED ossp-uuid)
    pkg_check_modules(JANSSON REQUIRED jansson)
ENDIF ()

#添加所有的源文件
SET(SRC
        base64.c
        KcCrc.c
        KcDateTime.c
        KcIpV4.c
        KcJson.c
        KcLog.c
        KcRandomString.c
        KcString.c
        KcUdp.c
        KcVld.c
        stringbuffer.c
)
#生成动态链接库
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC})

#生成可执行文件
#ADD_EXECUTABLE(${PROJECT_NAME}
#        ${SRC}
#)

#添加导出宏UKCEXPORTS
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME}
            PRIVATE
            UKCEXPORTS
            )
ENDIF ()

# 链接库文件
IF (WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            jansson::jansson
            ws2_32
            ${TRE_LIB}
            ${GETTIMEOFDAY_LIB}
            ${Iconv_LIBRARIES}
            ${Intl_LIBRARIES}
            )
ELSE ()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            jansson
            curses
            -lpthread
            -lm
            ${Iconv_LIBRARIES}
            ${Intl_LIBRARIES}
            )
ENDIF ()



# Installation
SET(install_targets ${install_targets} ${PROJECT_NAME})

#设置要安装的头文件
SET(KMBLACK_PUBLIC_HEADERS
        base64.h
        instr_time.h
        KcCore.h
        KcIpV4.h
        KcRegex.h
        KcShared.h
        KcSqlite3.h
        KcStdio.h
        KcUtility.h
        pg_bswap.h
        stringbuffer.h
        )

INSTALL(TARGETS ${install_targets}
        #可执行二进制文件
        RUNTIME DESTINATION bin
        #动态库
        LIBRARY DESTINATION lib
        #静态库
        ARCHIVE DESTINATION lib
        )

IF (INSTALL_PDB)
    INSTALL(FILES ${install_bin_pdb}
            DESTINATION bin
            CONFIGURATIONS RelWithDebInfo Debug)
ENDIF ()

INSTALL(FILES ${KMBLACK_PUBLIC_HEADERS} DESTINATION include)


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
