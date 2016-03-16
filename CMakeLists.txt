cmake_minimum_required(VERSION 2.6.0)

set(CATKIN_CALL "${CATKIN_DEVEL_PREFIX}")
if(CATKIN_CALL)
	message("Compiling signal scope as a catkin project")
	find_package(catkin REQUIRED)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
	set(PYTHONQT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../")
	set(CTK_PYTHONCONSOLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../")
	execute_process(COMMAND make externals WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

set(POD_NAME signal_scope)
include(cmake/pods.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")


find_package(PkgConfig)
pkg_check_modules(DRCLCMTYPES drc-lcmtypes)
set(use_builtin_lcmtypes_default OFF)
if(DRCLCMTYPES_FOUND)
  set(use_builtin_lcmtypes_default ON)
endif()


option(USE_BUILTIN_LCMTYPES "Compile builtin LCM types" ${use_builtin_lcmtypes_default})
option(USE_PYTHON "Enable Python support." ON)

add_subdirectory(src/signal_scope)

