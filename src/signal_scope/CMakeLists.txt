if("${USED_QTVERSION}" STREQUAL "5")
  find_package(Qt5 REQUIRED Core Gui Script Widgets)
  set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Script Qt5::Widgets)
  # Compatibility macros
  macro(qt4_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()
  macro(qt4_wrap_ui)
    qt5_wrap_ui(${ARGN})
  endmacro()
  macro(qt4_add_resources)
    qt5_add_resources(${ARGN})
  endmacro()
  if(APPLE)
    find_library(QWT_LIBRARY qwt)
  else()
    find_library(QWT_LIBRARY qwt-qt5)
  endif()
else()
  find_library(QWT_LIBRARY qwt)
  find_package(Qt4 REQUIRED QtCore QtGui QtScript)
  include(${QT_USE_FILE})
endif()

#find_package(LCM REQUIRED)

set(deps)

if(USE_BUILTIN_LCMTYPES)

  if(NOT DRCLCMTYPES_FOUND)
    message(SEND_ERROR "USE_BUILTIN_LCMTYPES is enabled but drc lcmtypes is not found.")
  endif()

  link_directories(${DRCLCMTYPES_LIBRARY_DIRS})
  include_directories(${DRCLCMTYPES_INCLUDE_DIRS})
  list(APPEND deps ${DRCLCMTYPES_LIBRARIES})
  add_definitions(-DUSE_BUILTIN_LCMTYPES)
endif()


find_path(QWT_INCLUDE_DIR qwt/qwt.h HINTS /usr/local/lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${QWT_INCLUDE_DIR})
include_directories(${LCM_INCLUDE_DIRS})

if(USE_PYTHON)

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
  find_package(PythonQt REQUIRED)
  find_package(CTKPythonConsole REQUIRED)

  include_directories(${PYTHON_INCLUDE_DIRS})
  include_directories(${PYTHONQT_INCLUDE_DIRS})
  include_directories(${CTK_PYTHONCONSOLE_INCLUDE_DIRS})

  set(python_deps ${PYTHON_LIBRARIES} ${CTK_PYTHONCONSOLE_LIBRARIES} ${PYTHONQT_LIBRARIES})
  set(python_moc_srcs
    #pythonsignalhandler.h
    #pythonchannelsubscribercollection.h
    #pythonchannelsubscriber.h
    #pythonmessageinspector.h
    )
  set(python_srcs)
endif()

qt4_wrap_cpp(moc_srcs
  mainwindow.h
  plot.h
  plotwidget.h
  #lcmthread.h
  #lcmsubscriber.h
  signalhandler.h
  setscaledialog.h
  selectsignaldialog.h
  ${python_moc_srcs}
  )

qt4_wrap_ui(ui_srcs
  mainwindow.ui
  setscaledialog.ui
  selectsignaldialog.ui
  )

qt4_add_resources(resource_srcs
  resources.qrc
  )

set(srcs
  ${moc_srcs}
  ${ui_srcs}
  ${resource_srcs}
  curvedata.cpp
  jointnames.cpp
  main.cpp
  mainwindow.cpp
  plot.cpp
  plotwidget.cpp
  setscaledialog.cpp
  signaldata.cpp
  signaldescription.cpp
  #lcmthread.cpp
  #lcmsubscriber.cpp
  signalhandler.cpp
  selectsignaldialog.cpp
  ${python_srcs}
  )

if (USE_BUILTIN_LCMTYPES)
  list(APPEND srcs builtinmessages.cpp)
endif()

add_executable(signal-scope ${srcs})

if (USE_BUILTIN_LCMTYPES)
  pods_use_pkg_config_packages(signal-scope lcmtypes_bot2-core)
endif()

list(APPEND deps
  ${QT_LIBRARIES}
  ${QWT_LIBRARY}
  ${python_deps}
  #${LCM_LIBRARIES}
  )

target_link_libraries(signal-scope ${deps})


pods_install_headers(
  curvedata.h
  lcmthread.h
  mainwindow.h
  plot.h
  plotwidget.h
  samplingthread.h
  selectsignaldialog.h
  setscaledialog.h
  signaldata.h
  signaldescription.h
  signalhandler.h
  ${python_moc_srcs}
  DESTINATION signal_scope)


pods_install_executables(signal-scope)

