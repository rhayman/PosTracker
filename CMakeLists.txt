#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

find_package( OpenCV 4.1.0 REQUIRED core highgui xfeatures2d features2d imgproc )

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	PosTracker/CameraBase.h
	PosTracker/Camera.h
	PosTracker/CameraCV.cpp
	PosTracker/CameraCV.h
	PosTracker/PosTracker.cpp
	PosTracker/PosTracker.h
	PosTracker/PosTrackerEditor.cpp
	PosTracker/PosTrackerEditor.h
	common.h
	)

target_link_libraries(${PLUGIN_NAME} ${OpenCV_LIBS})
plugin_create_filters()
