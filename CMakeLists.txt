#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

find_package( OpenCV REQUIRED core highgui imgproc tracking )

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	PosTracker/CameraBase.h
	PosTracker/Camera.h
	PosTracker/Camera.cpp
	PosTracker/CameraCV.cpp
	PosTracker/CameraCV.h
	PosTracker/PosTracker.cpp
	PosTracker/PosTracker.h
	PosTracker/PosTrackerEditor.cpp
	PosTracker/PosTrackerEditor.h
	cvTracking/cvTrackers.cpp
	cvTracking/cvTrackers.hpp
	cvTracking/Trackers.cpp
	cvTracking/Trackers.hpp
	cvTracking/TrackersEditor.cpp
	cvTracking/TrackersEditor.hpp
	StereoPos/StereoPos.cpp
	StereoPos/StereoPos.h
	StereoPos/StereoPosEditor.cpp
	StereoPos/StereoPosEditor.h
	common.h
	)

target_link_libraries(${PLUGIN_NAME} ${OpenCV_LIBS})
plugin_create_filters()
