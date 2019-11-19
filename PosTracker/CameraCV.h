#ifndef CAMERACV_H_
#define CAMERACV_H_

#include "CameraBase.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>

class CameraCV : public CameraBase
{
public:
	CameraCV() : CameraBase("openCV") {};
	CameraCV(std::string _dev_name) : CameraBase(_dev_name) {};
	~CameraCV();

	std::vector<Formats*> get_formats() override;

	int open_device() override;
	void close_device() override;
	int init_device() override;
	void uninit_device() override;
	int start_device() override;
	int stop_device() override;
	/*
	Reads the buffer and returns an openCV matrix & a timeval struct
	(in time.h) of the time when the first data byte was captured
	*/
	int read_frame(cv::Mat &, struct timeval &) override;

	int get_control_values(__u32, __s32 &, __s32 &, __s32 &) override;
	int set_control_value(__u32, int) override;
	int switch_exposure_type(int) override;
	int set_format() override; // overloads below method using zero index
	int set_format(const unsigned int /* index into availableFormats */) override;
	int set_format(const std::string /* overloads with string of format from Formats::get_description()*/) override;
	int set_format(const Formats * /* overloads with Format*/) override;
	int set_framesize(const unsigned int w, const unsigned int h) override;
	int set_framerate(const unsigned int fps) override;

	std::string get_format_name() override;

private:
	cv::VideoCapture cap;
	std::string dev_name = "openCV";
	// Default a list of Formats under openCV as there is no 'easy' way to
	// interrogate all the available formats, framerates, frame widths/ heights
	// etc etc according to the interwebs
	// TODO
};

#endif