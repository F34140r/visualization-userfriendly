# Build file for Python bindings via shiboken and pyside.

set(rviz_shiboken_QT_COMPONENTS
  QtCore
  QtGui
)

rosbuild_include(python_qt_binding shiboken_helper)

if(shiboken_helper_FOUND)
  list(APPEND rviz_BINDINGS "shiboken")
  set(rviz_BINDINGS "${rviz_BINDINGS}" PARENT_SCOPE)

  # To add a new rviz class to the bindings, add it:
  # - in rviz_shiboken_SRCS below, like rviz_myclass_wrapper.cpp
  # - in rviz_HDRS below, like ../rviz/my_class.h
  # - in global.h, like #include <rviz/my_class.h>
  # - in typesystem.xml, like <object-type name="rviz::MyClass"/>

  set(rviz_shiboken_SRCS
    librviz_shiboken/librviz_shiboken_module_wrapper.cpp
    librviz_shiboken/rviz_visualizationframe_wrapper.cpp
    librviz_shiboken/rviz_visualizationmanager_wrapper.cpp
    librviz_shiboken/rviz_display_wrapper.cpp
    librviz_shiboken/rviz_displaygroup_wrapper.cpp
    librviz_shiboken/rviz_ogrelogging_wrapper.cpp
    librviz_shiboken/rviz_property_wrapper.cpp
    librviz_shiboken/rviz_viewmanager_wrapper.cpp
    librviz_shiboken/rviz_viewcontroller_wrapper.cpp
    librviz_shiboken/rviz_tool_wrapper.cpp
    librviz_shiboken/rviz_toolmanager_wrapper.cpp
    librviz_shiboken/rviz_wrapper.cpp
  )

  set(rviz_shiboken_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)

  set(rviz_HDRS
    ${rviz_shiboken_INCLUDE_DIR}/rviz/visualization_frame.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/visualization_manager.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/view_manager.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/view_controller.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/display.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/display_group.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/properties/property.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/tool.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/tool_manager.h
    ${rviz_shiboken_INCLUDE_DIR}/rviz/ogre_helpers/ogre_logging.h
  )

  shiboken_generator(
    librviz
    global.h
    typesystem.xml
    ${PROJECT_SOURCE_DIR}/src/python_bindings/shiboken
    "${rviz_shiboken_SRCS}"
    "${rviz_HDRS}"
    "${rviz_shiboken_INCLUDE_DIR}"
    "${PROJECT_SOURCE_DIR}/build/src/python_bindings/shiboken"
  )

  include_directories(rviz_shiboken
    ${rviz_shiboken_INCLUDE_DIR}/rviz
    ${rviz_shiboken_INCLUDE_DIR}/rviz/properties
    ${rviz_shiboken_INCLUDE_DIR}/rviz/ogre_helpers
  )
  shiboken_include_directories(rviz_shiboken "${rviz_shiboken_QT_COMPONENTS}")
  rosbuild_add_library(rviz_shiboken ${rviz_shiboken_SRCS})
  target_link_libraries(rviz_shiboken ${PROJECT_NAME})
  shiboken_target_link_libraries(rviz_shiboken "${rviz_shiboken_QT_COMPONENTS}")

endif()
