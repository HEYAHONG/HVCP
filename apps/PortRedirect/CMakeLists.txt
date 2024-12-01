cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME PortRedirect)
project(${PROJECT_NAME}
	VERSION 1.0.0.0
	LANGUAGES C CXX
	DESCRIPTION https://github.com/HEYAHONG/HVCP.git)

if(CYGWIN)
set(BUILD_HVCP TRUE)
endif()
if(WIN32)
	if(MSVC)
		add_compile_options(-utf-8 )
	endif()
	set(BUILD_HVCP TRUE)
endif()

if(NOT BUILD_HVCP)
	message(FATAL_ERROR "only support windows!")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
file(GLOB C_CXX_FILES *.h *.cpp *.c *.hpp *.cxx ../../lib/*.h ../../lib/*.cpp ../../lib/*.c 
../3rdparty/libserialport/libserialport.h 
../3rdparty/libserialport/serialport.c 
../3rdparty/libserialport/timing.c 
../3rdparty/libserialport/windows.c
)
add_executable(${PROJECT_NAME}  ${C_CXX_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/window_resources.rc)
if(MSVC)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()
target_link_libraries(${PROJECT_NAME} setupapi cfgmgr32 ole32)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../lib  ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/libserialport)
#添加线程库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT})
target_compile_options(${PROJECT_NAME} PUBLIC -DLIBSERIALPORT_MSBUILD=1)

#添加3rdparty/HCppBox
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/HCppBox/master HCppBox EXCLUDE_FROM_ALL)
#启用HCppBox、HRC
hcppbox_enable(${PROJECT_NAME})
hrc_enable(${PROJECT_NAME})


include(GNUInstallDirs)
install(
	TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
