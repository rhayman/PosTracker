#include <iostream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "common.h"

#include "Camera.h"

Camera::~Camera()
{
	if (n_buffers > 0)
	{
		for (int i = 0; i < n_buffers; i++)
		{
			if ( -1 == munmap(buffers[i].start, buffers[i].length))
				errno_exit("munmap");
		}
	}
	if ( fd > -1 )
		close(fd);
}

int Camera::set_framesize(const unsigned int width, const unsigned int height)
{
	struct v4l2_format fmt;
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( -1 == xioctl(fd, VIDIOC_G_FMT, &fmt) )
		errno_exit("Could not get parameters");

	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;

	if ( -1 == xioctl(fd, VIDIOC_S_FMT, &fmt) )
		errno_exit("Cannot set frame size on the device");
	else
	{
		std::cout << "Set frame size to " << width << "x" << height;
		std::cout << " (w x h) on " << dev_name << std::endl;
	}

	return 0;
}

int Camera::set_framerate(const unsigned int fps)
{
	struct v4l2_streamparm streamparm;
	CLEAR(streamparm);
	streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( -1 == xioctl(fd, VIDIOC_G_PARM, &streamparm) )
		errno_exit("Cannot set parameters on the device");

	if ( streamparm.parm.capture.capability == V4L2_CAP_TIMEPERFRAME )
	{
		streamparm.parm.capture.timeperframe.numerator = 1;
		streamparm.parm.capture.timeperframe.denominator = fps;
		if ( -1 == xioctl(fd, VIDIOC_S_PARM, &streamparm) )
			errno_exit("Cannot set frame rate on device");
		else
		{
			std::cout << "Set frame rate on " << dev_name << " to ";
			std::cout << (double)fps << " fps " << std::endl;
		}

	}
	else
		errno_exit("Cannot set frame rate on device");
	return 0;
}

int Camera::set_format()
{
	unsigned int idx = 0;
	set_format(idx);
}

int Camera::set_format(const unsigned int index)
{
	if ( availableFormats.empty() )
		get_formats();

	Formats * thisfmt = availableFormats.at(index);
	// attempt to set frame size
	struct v4l2_format fmt;
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( -1 == xioctl(fd, VIDIOC_G_FMT, &fmt) )
		errno_exit("Could not get parameters");

	fmt.fmt.pix.width = thisfmt->width;
	fmt.fmt.pix.height = thisfmt->height;
	fmt.fmt.pix.pixelformat = thisfmt->pixelformat;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	// thisfmt.print();

	if ( -1 == xioctl(fd, VIDIOC_S_FMT, &fmt) )
	{
		errno_exit("Cannot set frame size on the device");
		return 1;
	}
	else
	{
		std::cout << "Set frame size to " << thisfmt->width << "x" << thisfmt->height;
		std::cout << " (w x h) on " << dev_name << std::endl;
	}

	/* Buggy driver paranoia. */
	unsigned int min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
			fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;

	// attempt to set frame rate
	struct v4l2_streamparm streamparm;
	CLEAR(streamparm);
	streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( -1 == xioctl(fd, VIDIOC_G_PARM, &streamparm) )
		errno_exit("Cannot set parameters on the device");

	if ( streamparm.parm.capture.capability == V4L2_CAP_TIMEPERFRAME )
	{
		streamparm.parm.capture.timeperframe.numerator = thisfmt->numerator;
		streamparm.parm.capture.timeperframe.denominator = thisfmt->denominator;
		if ( -1 == xioctl(fd, VIDIOC_S_PARM, &streamparm) )
			errno_exit("Cannot set frame rate on device");
		else
		{
			std::cout << "Set frame rate to " << (double)thisfmt->denominator / (double)thisfmt->numerator;
			std::cout << " fps on " << dev_name << std::endl;
		}

	}
	else
		errno_exit("Cannot set frame rate on device");
	currentFmt = thisfmt;
	std::cout << *currentFmt << std::endl;
	return 0;

}

int Camera::set_format(const std::string format)
{
	std::vector<std::string> allFormats = get_format_descriptions();
	for (int i = 0; i < allFormats.size(); ++i)
	{
		if ( format.compare(allFormats.at(i)) == 0)
			set_format(i);
	}
	return 0;
}

int Camera::set_format(const Formats * format)
{
	std::vector<Formats*> formats = get_formats();
	for (int i = 0; i < formats.size(); ++i)
	{
		if ( format == formats.at(i) )
			set_format(i);
	}
	return 0;
}

std::string Camera::get_dev_name()
{
	return dev_name;
}

std::string Camera::get_format_name()
{
	if ( currentFmt )
		return currentFmt->get_description();
	else
		return "";
}

std::vector<Formats*> Camera::get_formats()
{
	availableFormats.clear();

	struct v4l2_fmtdesc fmt;
	CLEAR(fmt);
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmt))
	{
		struct v4l2_frmsizeenum frmsize;
		CLEAR(frmsize);
		frmsize.index = 0;
		frmsize.pixel_format = fmt.pixelformat;

		while ( 0 == xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) )
		{
			struct v4l2_frmivalenum intervals;
			CLEAR(intervals);
			intervals.index = 0;
			intervals.pixel_format = frmsize.pixel_format;

			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
			{
				intervals.width = frmsize.discrete.width;
				intervals.height = frmsize.discrete.height;
				while ( 0 == xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &intervals) )
				{
					Formats * thisfmt = new Formats();
					thisfmt->index = fmt.index;
					thisfmt->stream_type = buf_type.at(fmt.type);
					thisfmt->description = charcode2str(fmt.description);
					thisfmt->pixelformat = fmt.pixelformat;

					thisfmt->framesize_type = frm_type.at(frmsize.type);

					thisfmt->numerator = intervals.discrete.numerator;
					thisfmt->denominator = intervals.discrete.denominator;
					thisfmt->discrete_frmsizes = frmsize.discrete;
					thisfmt->width = frmsize.discrete.width;
					thisfmt->height = frmsize.discrete.height;
					availableFormats.push_back(thisfmt);
					intervals.index++;
				}
			}
			else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
			{
				// if stepwise, get the min and max x,y and the steps
				intervals.width = frmsize.stepwise.min_width;
				intervals.height = frmsize.stepwise.min_height;
				while ( 0 == xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &intervals) )
				{
					Formats * thisfmt = new Formats();
					thisfmt->index = fmt.index;
					thisfmt->stream_type = buf_type.at(fmt.type);
					thisfmt->description = charcode2str(fmt.description);
					thisfmt->pixelformat = fmt.pixelformat;

					thisfmt->framesize_type = frm_type.at(frmsize.type);
					thisfmt->stepwise_intervals = intervals.stepwise;
					thisfmt->numerator = intervals.stepwise.min.numerator;
					thisfmt->denominator = intervals.stepwise.min.denominator;
					thisfmt->stepwise_frmsizes = frmsize.stepwise;
					thisfmt->width = frmsize.stepwise.max_width;
					thisfmt->height = frmsize.stepwise.max_height;
					availableFormats.push_back(thisfmt);
					intervals.index++;

				}
			}
			else
				fprintf(stderr, "Continuous framesize not supported for: %s\n", dev_name.c_str());
			frmsize.index++;	
		}
		fmt.index++;
	}
	return availableFormats;
}
int Camera::open_device()
{
	fprintf(stdout, "Attempting to open %s\n", dev_name.c_str());
	struct stat st;

	if (-1 == stat(dev_name.c_str(), &st))
	{
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
				 dev_name.c_str(), errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode))
	{
		fprintf(stderr, "%s is no devicen", dev_name.c_str());
		exit(EXIT_FAILURE);

	}

	fd = open(dev_name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd)
	{
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
				 dev_name.c_str(), errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Successfully opened %s\n", dev_name.c_str());

	is_ready = true;

	return 0;
}

int Camera::init_device()
{
	struct v4l2_capability cap;
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			fprintf(stderr, "%s is no V4L2 device\\n",
					 dev_name.c_str());
			exit(EXIT_FAILURE);
		}
		else
			errno_exit("VIDIOC_QUERYCAP");
	}
	if ( ! (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) )
	{
		fprintf(stderr, "%s is no video capture device\\n",
				 dev_name.c_str());
		exit(EXIT_FAILURE);
	}
	init_mmap();

	is_initialized = true;
}

int Camera::start_device()
{
	enum v4l2_buf_type type;
	for (unsigned int i = 0; i < n_buffers; ++i)
	{
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if ( -1 == xioctl(fd, VIDIOC_QBUF, &buf) )
			errno_exit("VIDIOC_QBUF");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if ( -1 == xioctl(fd, VIDIOC_STREAMON, &type) )
		errno_exit("VIDIOC_STREAMON");
	has_started = true;
}

int Camera::stop_device()
{
	if ( has_started == true )
	{
		enum v4l2_buf_type type;
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if ( -1 == xioctl(fd, VIDIOC_STREAMOFF, &type) )
			errno_exit("VIDIOC_STREAMOFF");
	}
	has_started = false;
	return 0;
}

int Camera::read_frame(cv::Mat & result, struct timeval &tv)
{
	fd_set fds;
	int r;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select(fd + 1, &fds, NULL, NULL, &tv);

	if ( -1 == r )
	{
		if (EINTR == errno)
		{
			errno_exit("select");
			result = cv::Mat();
			return 1;
		}
	}

	if ( 0 == r )
	{
		fprintf(stderr, "select timeout\n");
		exit(EXIT_FAILURE);
		result = cv::Mat();
		return 1;
	}

	struct v4l2_buffer buf;
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.flags = V4L2_BUF_FLAG_TIMECODE || V4L2_BUF_FLAG_TIMESTAMP_MASK && V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;

	if ( -1 == xioctl(fd, VIDIOC_DQBUF, &buf) )
	{
		switch (errno)
		{
			case EAGAIN:
			{
				errno_exit("EAGAIN");
				result = cv::Mat();
				return 1;
			}
			case EIO:
			// ignorable - see example at:
			// https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/capture.c.html
			default:
			{
				errno_exit("VIDIOC_DQBUF");
				result = cv::Mat();
				return 1;
			}
		}
	}
	tv = buf.timestamp;


	assert(buf.index < n_buffers);
	if (currentFmt->pixelformat == V4L2_PIX_FMT_YUYV)
	{
		result = cv::Mat(currentFmt->height, currentFmt->width, CV_8UC2, buffers[buf.index].start);
		cv::cvtColor(result, result, cv::COLOR_YUV2BGR_YUY2);
	}
	else if (currentFmt->pixelformat == V4L2_PIX_FMT_MJPEG)
	{
		result = cv::Mat(currentFmt->height, currentFmt->width, CV_8U);
		cv::imdecode(cv::Mat(1, currentFmt->height * currentFmt->width, CV_8U, (unsigned char *)buffers[buf.index].start), cv::IMREAD_COLOR, &result);
	}
	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	{
		result = cv::Mat();
		errno_exit("VIDIOC_QBUF");
		return 1;
	}
	return 0;
}

cv::Mat Camera::process_image(void *p, int size, struct timeval tv)
{}

void Camera::close_device()
{
	fprintf(stdout, "Closing %s\n", dev_name.c_str());
	if ( -1 == close(fd) )
		errno_exit("close");
	fd = -1;
	is_ready = false;
}

int Camera::switch_exposure_type(int autoOrManual)
{
	struct v4l2_control control;
	CLEAR(control);
	control.id = V4L2_CID_EXPOSURE_AUTO;
	control.value = autoOrManual;
	if ( -1 == xioctl(fd, VIDIOC_S_CTRL, &control) )
		errno_exit("VIDIOC_S_CTRL");
	return 0;
}

int Camera::set_control_value(__u32 id, int val)
{
	struct v4l2_queryctrl queryctrl;
	CLEAR(queryctrl);
	queryctrl.id = id;
	if ( -1 == xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) )
	{
		std::cout << "Could not set control value" << std::endl;
		errno_exit("VIDIOC_QUERYCTRL");
	}
	struct v4l2_control control;
	CLEAR(control);
	control.id = id;
	if ( val > queryctrl.maximum )
		val = queryctrl.maximum;
	if ( val < queryctrl.minimum )
		val = queryctrl.minimum;
	control.value = val;
	if ( -1 == xioctl(fd, VIDIOC_S_CTRL, &control) )
		errno_exit("VIDIOC_S_CTRL");
	return 0;
}

int Camera::get_control_values(__u32 id, __s32 & min, __s32 & max, __s32 & step)
{
	struct v4l2_queryctrl queryctrl;
	CLEAR(queryctrl);
	queryctrl.id = id;

	if ( -1  == xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
	{
		std::cout << "Could not query control" << std::endl;
		return 1;
	}

	min = queryctrl.minimum;
	max = queryctrl.maximum;
	step = queryctrl.step;
	return 0;
}

void Camera::init_mmap()
{
	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			fprintf(stderr, "%s does not support "
					 "memory mappingn", dev_name.c_str());
			exit(EXIT_FAILURE);
		}
		else
			errno_exit("VIDIOC_REQBUFS");
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\\n",
				 dev_name.c_str());
		exit(EXIT_FAILURE);
	}
	n_buffers = req.count;
	buffers = (buffer*)calloc(n_buffers, sizeof(*buffers));

	if (!buffers)
	{
		fprintf(stderr, "Out of memory\\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
				errno_exit("VIDIOC_QUERYBUF");

		buffers[i].length = buf.length;
		buffers[i].start =
				mmap(NULL /* start anywhere */,
					  buf.length,
					  PROT_READ | PROT_WRITE /* required */,
					  MAP_SHARED /* recommended */,
					  fd, buf.m.offset);

		if (MAP_FAILED == buffers[i].start)
				errno_exit("mmap");
	}
}

void Camera::uninit_device()
{
	if ( is_initialized )
	{
		for (int i = 0; i < n_buffers; ++i)
		{
			if ( -1 == munmap(buffers[i].start, buffers[i].length) )
				errno_exit("munmap");
		}
		free(buffers);
	}
	is_initialized = false;
}

std::vector<std::string> Camera::get_devices()
{
	std::vector<std::string> device_list;
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
	return device_list;
}