cmake_minimum_required(VERSION 3.11)

#设置项目名
PROJECT(regex VERSION 1.0.0 LANGUAGES C CXX)

#设置编译器标准
SET(CMAKE_C_STANDARD 99)

IF (UNIX)
	find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/include NO_DEFAULT_PATH)
	find_library(KMBLACK_LIB libkmblack.so ${KMBLACK_DIR}/lib)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(JANSSON REQUIRED jansson)
ELSE ()	
	find_package(Intl REQUIRED)
	find_package(jansson CONFIG REQUIRED)
	find_library(TRE_LIB tre.lib)
	find_path(KMBLACK_INCLUDE NAMES KcCore.h PATHS ${KMBLACK_DIR}/kmblack NO_DEFAULT_PATH)
	find_library(KMBLACK_LIB kmblack.lib "${KMBLACK_DIR}/x64/Release")
ENDIF ()

#添加所有的源文件
SET(SRC 
	main.c
)

#导入头文件目录
INCLUDE_DIRECTORIES(${KMBLACK_INCLUDE})

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC})

#为target添加需要链接的共享库
IF (WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE  jansson::jansson ${TRE_LIB} ${Intl_LIBRARIES}  ${KMBLACK_LIB})
ELSE ()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}  PRIVATE jansson ${Intl_LIBRARIES}  ${KMBLACK_LIB})
ENDIF ()
