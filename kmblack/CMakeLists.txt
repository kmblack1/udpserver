cmake_minimum_required(VERSION 3.11)

#设置项目名
project(kmblack VERSION 1.0.0 LANGUAGES C CXX)

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
        base64.c
        stringbuffer.c
        vld.c
)

#生成动态链接库
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC})

#生成可执行文件
#ADD_EXECUTABLE(${PROJECT_NAME}
#        ${SRC}
#)

#为target添加需要链接的共享库



# Installation
SET(install_targets ${install_targets} ${PROJECT_NAME})

#设置要安装的头文件
SET(KMBLACK_PUBLIC_HEADERS
        base64.h
        instr_time.h
        KcCore.h
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
