#ifndef CAMERA_H_
#define CAMERA_H_

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../common.h"
#include "CameraBase.h"

class Camera : public CameraBase
{
public:
	Camera() : CameraBase("/dev/video1")  {};
	Camera(std::string _dev_name) : CameraBase(_dev_name) {};
	~Camera();
	/*
	Returns a list of attached video devices e.g /dev/vided0 etc
	NB doesn't open them or assign fd
	*/
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
	void init_mmap();
};

#endif