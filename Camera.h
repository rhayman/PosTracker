#ifndef CAMERA_H_
#define CAMERA_H_

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "common.h"

class Camera
{
public:
	Camera() : dev_name("/dev/video1")  {};
	Camera(std::string _dev_name) : dev_name(_dev_name) { };
	~Camera();

	/*
	Returns a list of attached video devices e.g /dev/vided0 etc
	NB doesn't open them or assign fd
	*/
	static std::vector<std::string> get_devices();
	std::vector<Formats*> get_formats();
	Formats * get_current_format() { return currentFmt; };

	bool ready() { return is_ready; };
	bool initialized() { return is_initialized; }
	bool started() { return has_started; }

	int open_device();
	int init_device();
	void uninit_device();
	int start_device();
	/*
	Reads the buffer and returns an openCV matrix & a timeval struct
	(in time.h) of the time when the first data byte was captured
	*/
	int read_frame(cv::Mat &, struct timeval &);
	int stop_device();
	void close_device();

	int get_control_values(__u32, __s32 &, __s32 &, __s32 &);
	int set_control_value(__u32, int);
	int switch_exposure_type(int);
	int set_format(); // overloads below method using zero index
	int set_format(const unsigned int /* index into availableFormats */);
	int set_format(const std::string /* overloads with string of format from Formats::get_description()*/);
	int set_format(const Formats * /* overloads with Format*/);
	int set_framesize(const unsigned int w, const unsigned int h);
	int set_framerate(const unsigned int fps);

	void print_formats() {
		for (auto x : availableFormats)
			std::cout << x << std::endl;
	}

	std::vector<std::string> get_format_descriptions() {
		formatDescriptions.clear();
		for (auto x : availableFormats)
			formatDescriptions.push_back(x->get_description());
		return formatDescriptions;
	}

	int & getfd() { return fd; };
	std::string get_dev_name();
	std::string get_format_name();

private:
	void init_mmap();
	cv::Mat process_image(void *p, int size, struct timeval tv);
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