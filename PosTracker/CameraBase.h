#ifndef CAMERABASE_H_
#define CAMERABASE_H_

#include <iostream>
#include <string>
#include <vector>
#ifdef __unix__
#include <unistd.h>
#endif
#include "../common.h"

class CameraBase
{
public:
	CameraBase() : dev_name("/dev/video1")  {};
	CameraBase(std::string _dev_name) : dev_name(_dev_name) {};
	~CameraBase() {};
	/*
	Returns a list of attached video devices e.g /dev/vided0 etc
	NB doesn't open them or assign fd
	*/
	static std::vector<std::string> get_devices() {
		std::vector<std::string> device_list;
		#ifdef _WIN32
		cv::VideoCapture _cap;
		_cap.open(0);
		if ( _cap.isOpened() )
			device_list.push_back("0");
		#endif // _WIN32
		#ifdef __unix__
		unsigned int max_devices = 64;
		int fd = -1;
		for (int i = 0; i < max_devices; ++i)
		{
			char dev[64];
			sprintf(dev, "/dev/video%d", i);
			if ( -1 == (fd = open(dev, O_RDWR)))
				break;
			device_list.push_back(std::string(dev));
			close(fd);
		}
		#endif // __unix__
		return device_list;
	};
	virtual std::vector<Formats*> get_formats() {};
	Formats * get_current_format() { return currentFmt; };

	bool ready() { return is_ready; };
	bool initialized() { return is_initialized; }
	bool started() { return has_started; }

	virtual int open_device() {};
	virtual void close_device() {};
	virtual int init_device() {};
	virtual void uninit_device() {};
	virtual int start_device() {};
	virtual int stop_device() {};
	/*
	Reads the buffer and returns an openCV matrix & a timeval struct
	(in time.h) of the time when the first data byte was captured
	*/
	virtual int read_frame(cv::Mat &, struct timeval &) {};

	virtual int get_control_values(__u32, __s32 &, __s32 &, __s32 &) {};
	virtual int set_control_value(__u32, int) {};
	virtual int switch_exposure_type(int) {};
	virtual int set_format() {}; // overloads below method using zero index
	virtual int set_format(const unsigned int /* index into availableFormats */) {};
	virtual int set_format(const std::string /* overloads with string of format from Formats::get_description()*/) {};
	virtual int set_format(const Formats * /* overloads with Format*/) {};
	virtual int set_framesize(const unsigned int w, const unsigned int h) {};
	virtual int set_framerate(const unsigned int fps) {};

	std::vector<std::string> get_format_descriptions() {
		formatDescriptions.clear();
		for (auto x : availableFormats)
			formatDescriptions.push_back(x->get_description());
		return formatDescriptions;
	}

	int & getfd() { return fd; };
	std::string get_dev_name() { return dev_name; }
	virtual std::string get_format_name() {};

protected:
	int fd = -1;
	struct buffer *buffers;
	unsigned int n_buffers = 0;
	std::string dev_name ="";
	bool is_ready = false;
	bool is_initialized = false;
	bool has_started = false;

	Formats * currentFmt = nullptr;

	std::vector<Formats*> availableFormats;
	std::vector<std::string> formatDescriptions;


};

#endif