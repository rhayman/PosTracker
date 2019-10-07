#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../common.h"

#include "CameraCV.h"

CameraCV::~CameraCV() {
	if ( started() ) {
		stop_device();
		std::cout << "Stopped device\n";
	}
	if ( initialized() ) {
		uninit_device();
		std::cout << "Uninit device\n";
	}
	if ( ready() ) {
		close_device();
		std::cout << "Closed device\n";
	}
}

int CameraCV::set_framesize(const unsigned int width, const unsigned int height)
{
	if ( ! cap.set(cv::CAP_PROP_FRAME_WIDTH, width) )
		return 1;
	if ( ! cap.set(cv::CAP_PROP_FRAME_HEIGHT, height) )
		return 1;
	return 0;
}

int CameraCV::set_framerate(const unsigned int fps)
{
	if ( ! cap.set(cv::CAP_PROP_FPS, fps) )
		return 1;
	return 0;
}

int CameraCV::set_format()
{
	unsigned int idx = 0;
	return set_format(idx);
}

int CameraCV::set_format(const unsigned int index)
{
	if ( availableFormats.empty() )
		get_formats();

	Formats * thisfmt = availableFormats.at(index);
	// attempt to set frame size
	if ( set_framesize(thisfmt->width, thisfmt->height) ) {
		std::cout << "Set frame size to " << thisfmt->width << "x" << thisfmt->height;
		std::cout << " (w x h) on " << dev_name << std::endl;

		// currentFmt = thisfmt;
		// std::cout << *currentFmt << std::endl;
		return 0;
	}
	return 1;
}

int CameraCV::set_format(const std::string format)
{
	std::vector<std::string> allFormats = get_format_descriptions();
	for (int i = 0; i < allFormats.size(); ++i)
	{
		if ( format.compare(allFormats.at(i)) == 0)
			return set_format(i);
	}
	return 0;
}

int CameraCV::set_format(const Formats * format)
{
	std::vector<Formats*> formats = get_formats();
	for (int i = 0; i < formats.size(); ++i)
	{
		if ( format == formats.at(i) )
			return set_format(i);
	}
	return 0;
}

std::string CameraCV::get_format_name()
{
	if ( currentFmt )
		return currentFmt->get_description();
	else
		return "";
}

std::vector<Formats*> CameraCV::get_formats()
{
	availableFormats.clear();

	return availableFormats;
}

int CameraCV::open_device()
{
	fprintf(stdout, "Attempting to open %s\n", dev_name.c_str());
	if ( cap.open(0) ) {
		fprintf(stdout, "Successfully opened %s\n", dev_name.c_str());
		is_ready = true;
		return 0;
	}
	return 1;
}

int CameraCV::init_device()
{
	// CODE!

	is_initialized = true;

	return 0;
}

int CameraCV::start_device()
{
	// CODE!
	has_started = true;
	return 0;
}

int CameraCV::stop_device()
{
	if ( has_started == true )
	{
		// MORE CODE!
	}
	has_started = false;
	return 0;
}

int CameraCV::read_frame(cv::Mat & result, struct timeval &tv)
{
	cap >> result;
	return 0;
}

void CameraCV::close_device()
{
	// CLOSING CODE
	cap.release();
	fd = -1;
	is_ready = false;
}

int CameraCV::switch_exposure_type(int autoOrManual)
{
	cap.set(cv::CAP_PROP_AUTO_EXPOSURE, autoOrManual);
	return 0;
}

int CameraCV::set_control_value(__u32 id, int val)
{
	// CODE!
	return 0;
}

int CameraCV::get_control_values(__u32 id, __s32 & min, __s32 & max, __s32 & step)
{
	// CODE!
	return 0;
}

void CameraCV::uninit_device()
{
	if ( is_initialized )
	{
	}
	is_initialized = false;
}