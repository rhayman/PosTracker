#ifndef CAMERABASE_H_
#define CAMERABASE_H_

#include <iostream>
#include <string>
#include <vector>
#include <winsock.h>
#include "../common.h"

class CameraBase
{
public:
	CameraBase() : dev_name("")  {};
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
			device_list.push_back("cap0");
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
		Formats *  fmt{};
		std::vector<Formats*> dummy{};
		dummy.push_back(fmt);
		return dummy;
	}
	Formats * get_current_format() { return currentFmt; };

	bool ready() { return is_ready; };
	bool initialized() { return is_initialized; }
	bool started() { return has_started; }

	virtual int open_device() { return 1; }
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

	virtual int get_control_values(__u32, __s32 &, __s32 &, __s32 &) { return 1; }
	virtual int set_control_value(__u32, int) { return 1; }
	virtual int switch_exposure_type(int) { return 1; }
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
	std::string get_dev_name() { return dev_name; }
	virtual std::string get_format_name() { return ""; }

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