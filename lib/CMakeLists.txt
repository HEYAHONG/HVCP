cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME HVCP)
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
file(GLOB HVCP_C_CXX_FILES *.h *.cpp *.c *.hpp *.cxx)
add_library(${PROJECT_NAME} SHARED ${HVCP_C_CXX_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/window_resources.rc)
if(MSVC)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
endif()
target_link_libraries(${PROJECT_NAME} setupapi cfgmgr32 ole32)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include(GNUInstallDirs)
install(FILES HVCP.h
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
set(PROJECT_EXPORT_TARGETS ${PROJECT_NAME})
set(PROJECT_EXPORT_NAME ${PROJECT_NAME})
set(PROJECT_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_EXPORT_NAME})
foreach(target ${PROJECT_EXPORT_TARGETS})
	add_library(${PROJECT_EXPORT_NAME}::${target} ALIAS ${target})
endforeach()
install(
	TARGETS ${PROJECT_EXPORT_TARGETS}
	EXPORT ${PROJECT_EXPORT_NAME}Targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
export(
	EXPORT ${PROJECT_EXPORT_NAME}Targets
	NAMESPACE ${PROJECT_EXPORT_NAME}::
	FILE ${PROJECT_EXPORT_NAME}Targets.cmake
)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_EXPORT_NAME}Targets.cmake
	DESTINATION ${PROJECT_INSTALL_CMAKEDIR}
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${PROJECT_EXPORT_NAME}ConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_EXPORT_NAME}ConfigVersion.cmake
	DESTINATION ${PROJECT_INSTALL_CMAKEDIR}
)
install(
	EXPORT ${PROJECT_EXPORT_NAME}Targets
	FILE ${PROJECT_EXPORT_NAME}Config.cmake
	NAMESPACE ${PROJECT_EXPORT_NAME}::
	DESTINATION ${PROJECT_INSTALL_CMAKEDIR}
)

#测试
add_executable(HVCPEnum ${CMAKE_CURRENT_SOURCE_DIR}/test/HVCPEnum.cpp)
target_link_libraries(HVCPEnum ${PROJECT_NAME})

add_executable(HVCPRead ${CMAKE_CURRENT_SOURCE_DIR}/test/HVCPRead.cpp)
target_link_libraries(HVCPRead ${PROJECT_NAME})

add_executable(HVCPWrite ${CMAKE_CURRENT_SOURCE_DIR}/test/HVCPWrite.cpp)
target_link_libraries(HVCPWrite ${PROJECT_NAME})

add_executable(DriverAdd ${CMAKE_CURRENT_SOURCE_DIR}/test/DriverAdd.cpp)
target_link_libraries(DriverAdd ${PROJECT_NAME})


add_executable(DriverRemove ${CMAKE_CURRENT_SOURCE_DIR}/test/DriverRemove.cpp)
target_link_libraries(DriverRemove ${PROJECT_NAME})
