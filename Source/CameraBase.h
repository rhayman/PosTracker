#ifndef CAMERABASE_H_
#define CAMERABASE_H_

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "common.h"
#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <string>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <winsock.h>
#endif

struct v4l2_frmsize_stepwise {
	unsigned int step_width = 10;
	unsigned int step_height = 10;
	unsigned int max_width = 640;
	unsigned int max_height = 480;
	unsigned int min_height = 480;
	unsigned int min_width = 640;
};

struct v4l2_frmival_stepwise {
	struct min {
		unsigned int denominator = 30;
		unsigned int numerator = 1;
	};
	struct max {
		unsigned int denominator = 30;
		unsigned int numerator = 1;
	};
};

enum class TrackerType : int {
	kLED = 0,
	kTwoSpotTracking,
	kBoosting,
	kGOTURN,
	kKCF,
	kMedianFlow,
	kMIL,
	kMOSSE,
	kTLD,
	kBACKGROUND,
	kBACKGROUNDKNN
};

class Formats
{
public:
	unsigned int index; // 0,1,2,...
	std::string stream_type; // V4L2_BUF_TYPE_VIDEO_CAPTURE etc
	std::string description = "openCV"; // “YUV 4:2:2” etc
	unsigned int pixelformat; // four-character code e.g. 'YUYV'
	std::string framesize_type; // Discrete, step-wise or continuous
	struct v4l2_frmsize_stepwise stepwise_frmsizes;
	struct v4l2_frmival_stepwise stepwise_intervals;
	unsigned int numerator = 1;
	unsigned int denominator = 30;
	unsigned int width = 640;
	unsigned int height = 480;

	friend bool operator==(const Formats& lhs, const Formats& rhs)
	{
		return lhs.numerator == rhs.numerator && lhs.denominator == rhs.denominator &&
			lhs.width == rhs.width && lhs.height == rhs.height;
	}

	std::string get_resolution() { return std::to_string(width) + "x" + std::to_string(height); }
	std::string get_fps() { return std::to_string(int(denominator / numerator)) + " fps"; }
	unsigned int get_framerate() { return int(denominator / numerator); }
	std::string get_pixel_format() { return description; }
	std::string get_description() { return get_resolution() + " " + get_fps() + " " + get_pixel_format(); }
};

class CameraBase
{
public:
	CameraBase() : dev_name("cap0")  {};
	CameraBase(std::string _dev_name) : dev_name(_dev_name) {};
	~CameraBase() {};
	/*
	Returns a list of attached video devices e.g /dev/vided0 etc
	NB doesn't open them or assign fd
	*/
	static std::vector<std::string> get_devices() {
		std::vector<std::string> device_list;
		cv::VideoCapture _cap;
		_cap.open(0);
		if ( _cap.isOpened() )
			device_list.push_back("0");
		return device_list;
	};
	/*
	This is the method that under Linux populates the Container with valid formats
	by iterating over the V4L2 format enum. Here we give the default Format which
	should contain some sane values for resolution and fps etc. Apparently there
	is no straightforward way under openCV to retrieve these values given the number
	of APIs it has to support
	*/
	virtual std::vector<Formats*> get_formats() {
		availableFormats.clear();
		Formats *  fmt = new Formats();
		fmt->denominator = 30;
		fmt->numerator = 1;
		fmt->height = 480;
		fmt->width = 640;
		availableFormats.push_back(fmt);
		currentFmt = fmt;
		return availableFormats;
	}
	Formats * get_current_format() { return currentFmt; }

	bool ready() { return is_ready; };
	bool initialized() { return is_initialized; }
	bool started() { return has_started; }

	// Make this class abstract - maybe make more methods pure virtual to
	// make the concept clearer
	virtual int open_device() = 0; // pure virtual
	virtual void close_device() {};
	virtual int init_device() { return 1; }
	virtual void uninit_device() {};
	virtual int start_device() { return 1; }
	virtual int stop_device() { return 1; }
	/*
	Reads the buffer and returns an openCV matrix & a timeval struct
	(in time.h) of the time when the first data byte was captured
	*/
	virtual int read_frame(cv::Mat &, struct timeval &) { return 1; }
	virtual int get_control_values(const CamControl &, double &) { return 1; }
	virtual int set_control_value(const CamControl &, const int &) { return 1; }
	virtual int switch_exposure_type(const CamControl &) { return 1; }
	virtual int set_format() { return 1; } // overloads below method using zero index
	virtual int set_format(const unsigned int /* index into availableFormats */) { return 1; }
	virtual int set_format(const std::string /* overloads with string of format from Formats::get_description()*/) { return 1; }
	virtual int set_format(const Formats * /* overloads with Format*/) { return 1; }
	virtual int set_framesize(const unsigned int w, const unsigned int h) { return 1; }
	virtual int set_framerate(const unsigned int fps) { return 1; }

	std::vector<std::string> get_format_descriptions() {
		formatDescriptions.clear();
		for (auto x : availableFormats)
			formatDescriptions.push_back(x->get_description());
		return formatDescriptions;
	}

	int & getfd() { return fd; };

	virtual std::string get_format_name() { return ""; }
	std::string get_dev_name() { return dev_name; }

protected:
	int fd = -1;
	struct buffer *buffers;
	unsigned int n_buffers = 0;
	std::string dev_name = "";
	bool is_ready = false;
	bool is_initialized = false;
	bool has_started = false;
	Formats * currentFmt = nullptr;
	std::vector<Formats*> availableFormats;
	std::vector<std::string> formatDescriptions;
};

#endif
