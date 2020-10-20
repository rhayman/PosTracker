#ifndef COMMON_H_
#define COMMON_H_

#include <array>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

#ifdef __unix__
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#endif

#ifdef _WIN32
struct timeval
{
	std::string aww("Imma empty!");
};
#endif

#ifdef __unix__
	#define CLEAR(x) memset(&(x), 0, sizeof(x))

	struct buffer {
		void   *start;
		size_t  length;
	};

	static void errno_exit(const char *s)
	{
		fprintf(stderr, "%s error %d, %s\\n", s, errno, strerror(errno));
		exit(EXIT_FAILURE);
	};

	static void errno_exit(std::string s)
	{
		const char * c = s.c_str();
		fprintf(stderr, "%s error %d, %s\\n", c, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	static int xioctl(int fh, int request, void *arg)
	{
		int r;

		do
		{
			r = ioctl(fh, request, arg);
		}
		while (-1 == r && EINTR == errno);

		return r;
	};
#endif //__unix__

template <typename T>
std::string charcode2str(T & in)
{
	int sz = sizeof(in);
	char c[sz+1] = {0};
	strncpy(c, (char*)&in, sz);
	return std::string(c);
};

struct Formats
{
	__u32 index; // 0,1,2,...
	std::string stream_type; // V4L2_BUF_TYPE_VIDEO_CAPTURE etc
	std::string description; // “YUV 4:2:2” etc
	__u32 pixelformat; // four-character code e.g. 'YUYV'
	std::string framesize_type; // Discrete, step-wise or continuous
	struct v4l2_frmsize_discrete discrete_frmsizes;
	struct v4l2_frmsize_stepwise stepwise_frmsizes;
	struct v4l2_frmival_stepwise stepwise_intervals;
	__u32 numerator = 0;
	__u32 denominator = 0;
	__u32 width = 0;
	__u32 height = 0;

	friend std::ostream & operator<<(std::ostream & out, const Formats & fmt)
	{
		out << "\tIndex\t\t: " << fmt.index << "\n\tType\t\t: " << fmt.stream_type
			<< "\n\tDescription\t: " << fmt.description << "\n\tPixel format\t: " << charcode2str(fmt.pixelformat);
			if ( fmt.framesize_type == "Discrete")
			{
				out << "\n\t\tSize\t: " << fmt.framesize_type << " " << fmt.width << "x" << fmt.height
					<< "\n\t\tFramerate(denom/numer)\t: " << fmt.denominator << "/" << fmt.numerator << std::endl;  
			}
			else if ( fmt.framesize_type == "Step-wise" || fmt.framesize_type == "Continuous")
			{
				out << "\n\t\tSize\t: " << fmt.framesize_type << " " << fmt.stepwise_frmsizes.min_width << "x" << fmt.stepwise_frmsizes.min_height
					<< " - " << fmt.stepwise_frmsizes.max_width << "x" << fmt.stepwise_frmsizes.max_height
					<< " with step " << fmt.stepwise_frmsizes.step_width << "/" << fmt.stepwise_frmsizes.step_height
					<< "\n\tFramerate(denom/numer)\t: " << fmt.stepwise_intervals.max.denominator << "/" <<  fmt.stepwise_intervals.max.numerator
					<< " - " << fmt.stepwise_intervals.min.denominator << "/" << fmt.stepwise_intervals.min.numerator << std::endl;  
			}
		return out;
	}

	friend bool operator==(const Formats & lhs, const Formats & rhs)
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

enum class BORDER
{
	LEFT,
	RIGHT,
	TOP,
	BOTTOM
};

enum class IOMETHOD
{
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

enum class FMTFLAGS
{
	COMPRESSED = 1,
	EMULATED   = 2
};

// used for filling out list boxes or whatever JUCE calls em
static const std::vector<std::string> kTrackers {"LED", "Boosting", "KCF", "MedianFlow", "MIL", "MOSSE", "TLD", "Background", "BackgroundKNN" };
static const std::vector<std::string> kcf_modes {"GRAY", "COLOR"};

enum class TrackerType : int {
	kLED = 0,
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

const std::array<std::string, 14> buf_type = {
	"undefined",
	"video_capture",
	"video_output",
	"video_overlay",
	"vbi_capture",
	"vbi_output",
	"sliced_vbi_capture",
	"sliced_vbi_output",
	"video_output_overlay",
	"video_capture_mplane",
	"video_output_mplane",
	"sdr_capture",
	"sdr_output",
	"meta_capture"
};

const std::array<std::string, 4> frm_type = {
	"Undefined",
	"Discrete",
	"Step-wise",
	"Continuous"
};
#endif