#ifndef COMMON_H_
#define COMMON_H_

#include <array>
#include <string>
#include <cstring>
#include <cstdio>
#include <cerrno>

using __u32 = unsigned int;
using __s32 = int;
static constexpr unsigned int V4L2_EXPOSURE_AUTO = 1;
static constexpr unsigned int V4L2_EXPOSURE_MANUAL = 2;
static constexpr unsigned int V4L2_CID_BRIGHTNESS = 3;
static constexpr unsigned int V4L2_CID_CONTRAST = 4;
static constexpr unsigned int V4L2_CID_EXPOSURE_ABSOLUTE = 5;
static constexpr unsigned int CLOCK_MONOTONIC = 6;

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

template <typename T>
std::string charcode2str(T & in)
{
	int sz = sizeof(in);
	char c[sz+1] = {0};
	std::strncpy(c, (char*)&in, sz);
	return std::string(c);
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