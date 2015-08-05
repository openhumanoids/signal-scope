cmake_minimum_required(VERSION 2.8.5)
project(Superbuild)

# set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(ExternalProject)
set_property(DIRECTORY PROPERTY EP_PREFIX ${Superbuild_BINARY_DIR})

set(source_prefix ${Superbuild_BINARY_DIR}/src)
set(install_prefix ${CMAKE_INSTALL_PREFIX})


###############################################################################
# options
option(BUILD_LCM "Build LCM.  If OFF, then you must have LCM system installed." OFF)
option(BUILD_SIGNAL_SCOPE "Build signal-scope.  If OFF, only builds dependencies." OFF)


###############################################################################
# required dependencies
set(default_cmake_args
  "-DCMAKE_PREFIX_PATH:PATH=${install_prefix};${CMAKE_PREFIX_PATH}"
  "-DCMAKE_INSTALL_PREFIX:PATH=${install_prefix}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DBUILD_SHARED_LIBS:BOOL=ON"
  "-DBUILD_DOCUMENTATION:BOOL=OFF"
  "-DENABLE_TESTING:BOOL=OFF"
  )

# Find required external dependencies
find_package(Qt4 4.8 REQUIRED)

set(qt_args
  -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
  )

if(APPLE)
  find_program(PYTHON_CONFIG_EXECUTABLE python-config)
  if (NOT PYTHON_CONFIG_EXECUTABLE)
    message(SEND_ERROR "python-config executable not found, but python is required.")
  endif()
  # using "python-config --prefix" so that cmake always uses the python that is
  # in the users path, this is a fix for homebrew on Mac:
  # https://github.com/Homebrew/homebrew/issues/25118
  execute_process(COMMAND ${PYTHON_CONFIG_EXECUTABLE} --prefix OUTPUT_VARIABLE python_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(PYTHON_INCLUDE_DIR ${python_prefix}/include/python2.7)
  set(PYTHON_LIBRARY ${python_prefix}/lib/libpython2.7${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

find_package(PythonLibs 2.7 REQUIRED)

set(python_args
  -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
  -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR}
  -DPYTHON_LIBRARY:PATH=${PYTHON_LIBRARY}
  )


###############################################################################
# lcm
if (BUILD_LCM)

  ExternalProject_Add(
    lcm
    URL https://github.com/lcm-proj/lcm/releases/download/v1.2.1/lcm-1.2.1.zip
    URL_MD5 4355889a2367697a9458e83f6251f2b2
    CONFIGURE_COMMAND ${source_prefix}/lcm/configure --prefix=${install_prefix}
    BUILD_IN_SOURCE 1
  )

  set(lcm_depends lcm)

endif()


###############################################################################
# PythonQt
ExternalProject_Add(PythonQt
  GIT_REPOSITORY https://github.com/commontk/PythonQt.git
  GIT_TAG 00e6c6b2
  CMAKE_CACHE_ARGS
    ${default_cmake_args}
    ${qt_args}
    ${python_args}
    -DPythonQt_Wrap_Qtcore:BOOL=ON
    -DPythonQt_Wrap_Qtgui:BOOL=ON
    -DPythonQt_Wrap_Qtuitools:BOOL=ON
  )


###############################################################################
# ctkPythonConsole
ExternalProject_Add(ctkPythonConsole
  GIT_REPOSITORY https://github.com/patmarion/ctkPythonConsole
  GIT_TAG 15988c5
  CMAKE_CACHE_ARGS
    ${default_cmake_args}
    ${qt_args}
    ${python_args}
  DEPENDS
    PythonQt
  )


###############################################################################
# signal-scope
if (BUILD_SIGNAL_SCOPE)
  ExternalProject_Add(signal-scope
    SOURCE_DIR ${Superbuild_SOURCE_DIR}/..
    CMAKE_CACHE_ARGS
      ${default_cmake_args}
      ${qt_args}
      ${python_args}
    DEPENDS
      PythonQt
      ctkPythonConsole
      ${lcm_depends}
    )
endif()
