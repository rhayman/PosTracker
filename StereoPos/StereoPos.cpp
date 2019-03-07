#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <unistd.h>
#include <cuchar>
#include <ctime>
#include <algorithm>
#include <iterator>
#include "StereoPos.h"
#include "StereoPosEditor.h"
#include "../PosTracker/PosTracker.h"
#include "../PosTracker/Camera.h"

template<typename T>
using ioArray = std::vector<std::vector<T>>;

class CalibrateCamera {
public:
	CalibrateCamera() {};
	CalibrateCamera(double width, double height, double size, BOARDPROP bType) : m_width(width), m_height(height), m_size(size), m_board_type(bType) {};
	~CalibrateCamera() {};
	void setCameraName(std::string name) { m_camera_name = name; }
	void setup(std::vector<cv::Mat> imgs, bool showImages=false) {
		m_showImages = showImages;
		cv::Size board_size = cv::Size(m_width, m_height);
		std::vector<cv::Point2f> corners;
		cv::SimpleBlobDetector::Params params;
		params.minArea = 10;
		params.maxArea = 10e5;
		params.minDistBetweenBlobs = 5;
		cv::Ptr<cv::SimpleBlobDetector> blobDetector = cv::SimpleBlobDetector::create(params);
		m_object_points.clear();
		m_image_points.clear();
		m_image_ids.clear();
		// what goes into m_object_points
		std::vector<cv::Point3f> obj;
		for (int i = 0; i < m_height; ++i) {
			for (int j = 0; j < m_width; ++j)
				obj.push_back(cv::Point3f((float)j * m_size, (float)i * m_size, 0));
		}
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			m_img_size = grey.size();
			switch ( m_board_type ) {
				case BOARDPROP::kChessBoard: {
					m_found = cv::findChessboardCorners(grey, board_size, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FILTER_QUADS);
					break;
				}
				case BOARDPROP::kCircularSymmetric: {
					m_found = cv::findCirclesGrid(grey, board_size, corners, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);
					break;
				}
				case BOARDPROP::kCircularAsymmetric: {
					m_found = cv::findCirclesGrid(grey, board_size, corners, cv::CALIB_CB_ASYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);
					break;
				}
			}
			if ( m_found ) {
				std::cout << "Found a calibration pattern at position " << std::to_string(i) << std::endl;
				m_setup_success = true;
				m_image_points.push_back(corners);
				m_object_points.push_back(obj);
				m_image_ids.push_back(i);
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				if ( showImages ) {
					cv::Mat col;
					grey.copyTo(col);
					cv::cvtColor(col, col, cv::COLOR_GRAY2BGR);
					cv::drawChessboardCorners(col, board_size, corners, m_found);
					cv::imshow("Camera " + m_camera_name + ", pattern " + std::to_string(i), grey);
					char c = (char)cv::waitKey(0);
					if ( c == 27 || c == 'q') { // ESC or 'q' pressed
						cv::destroyWindow("Camera " + m_camera_name + ", pattern " + std::to_string(i));
					}
				}
			}
		}
	};

	bool calibrate() {
		int flag = 0;
		flag |= CV_CALIB_FIX_K4;
		flag |= CV_CALIB_FIX_K5;
		if ( ! m_object_points.empty() ) {
			cv::calibrateCamera(m_object_points, m_image_points, m_img_size, m_cameraMatrix, m_distCoeffs, m_rvecs, m_tvecs, flag);
			std::cout << "Successfully calibrated camera" << std::endl;
			return true;
		}
		else {
			std::cout << "No calibration patterns were found, try again..." << std::endl;
			return false;
		}
	};

	std::vector<int> getIDs() { return m_image_ids; }
	ioArray<cv::Point3f> getObjectPoints() { return m_object_points; }
	ioArray<cv::Point2f> getImagePoints() { return m_image_points; }
	cv::Mat getCameraMatrix() { return m_cameraMatrix; }
	cv::Mat getDistCoeffs() { return m_distCoeffs; }
	bool setup_success() { return m_setup_success; }
private:
	bool m_setup_success = false;
	bool m_found = false;
	bool m_showImages = false;
	double m_width = 11;
	double m_height = 12;
	double m_size = 11;
	BOARDPROP m_board_type = BOARDPROP::kChessBoard;
	// the size of the image(s) - this is set in setup()
	cv::Size m_img_size;
	// vectors for holding the object and image points (openCV parlance)
	ioArray<cv::Point3f> m_object_points; // holds the x / y corners of the chessboard (determined from m_size)
	ioArray<cv::Point2f> m_image_points; // holds the detected corners (from findChessboardCorners/ findCirclesGrid)
	std::vector<int> m_image_ids; // used to match corresponding images between two cameras
	/*
	 The actual calibration matrices:
	cameraMatrix - the intrinsic camera matrix (3x3)
	distCoeffs - the distortion coefficients for this camera of 4, 5, 8, 12 or 14 elements
	rvecs, tvecs - rotation and translation vectors estimated for each pattern view
	*/
	cv::Mat m_cameraMatrix, m_distCoeffs;
	std::vector<cv::Mat> m_rvecs, m_tvecs;
	std::string m_camera_name = "";
};

StereoPos::StereoPos() : GenericProcessor("Stereo Pos"), Thread("StereoPosThread")
{
	setProcessorType (PROCESSOR_TYPE_FILTER);
	sendSampleCount = false;
}

StereoPos::~StereoPos() {
	 for (int i = 0; i < calibrators.size(); ++i)
	 	calibrators[i].reset();
	 cv::destroyAllWindows();
}

void StereoPos::updateSettings()
{
	if ( editor != NULL )
		editor->updateSettings();
}

AudioProcessorEditor * StereoPos::createEditor() {
	editor = new StereoPosEditor(this, true);
	return editor;

}

void StereoPos::handleEvent(const EventChannel * eventInfo, const MidiMessage & event, int samplePosition) {
	std::cout << "eventInfo->getName() " << eventInfo->getName() << std::endl;
}

void StereoPos::showCapturedImages(bool show) {}

void StereoPos::calibrate(CalibrateCamera * camera_1, CalibrateCamera * camera_2) {
	// check for calibration patterns from both cameras at the same pattern position
	auto ids_1 = camera_1->getIDs();
	auto ids_2 = camera_2->getIDs();
	std::sort(ids_1.begin(), ids_1.end());
	std::sort(ids_2.begin(), ids_2.end());
	std::vector<int> id_intersection;
	std::set_intersection(ids_1.begin(), ids_1.end(), ids_2.begin(), ids_2.end(), std::back_inserter(id_intersection));

	if ( ! id_intersection.empty() ) {
		std::cout << "Captured calibration patterns at positions:\n";
		for (int n: id_intersection)
			std::cout << n << "\t" << std::endl;
		ioArray<cv::Point3f> obj_pts_1;
		ioArray<cv::Point2f> img_pts_1;
		ioArray<cv::Point3f> obj_pts_2;
		ioArray<cv::Point2f> img_pts_2;
		auto cam1_obj_pts = camera_1->getObjectPoints();
		auto cam1_img_pts = camera_1->getImagePoints();
		auto cam2_obj_pts = camera_2->getObjectPoints();
		auto cam2_img_pts = camera_2->getImagePoints();
		for (int n : id_intersection ) {
			std::vector<int> item{n};
			auto result = std::find_first_of(ids_1.begin(), ids_1.end(), item.begin(), item.end());
			auto idx = std::distance(ids_1.begin(), result);
			obj_pts_1.push_back(cam1_obj_pts[idx]);
			img_pts_1.push_back(cam1_img_pts[idx]);
			idx = std::distance(ids_2.begin(), result);
			obj_pts_2.push_back(cam2_obj_pts[idx]);
			img_pts_2.push_back(cam2_img_pts[idx]);
		}
	}
	else {
		std::cout << "No matching calibration patterns were captured on both cameras" << std::endl;
		return;
	}
}

void StereoPos::startStreaming() {
	// get some information abou the chessboard
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	nImagesToCapture = ed->getNImagesToCapture();
	double board_width = ed->getBoardDims(BOARDPROP::kWidth);
	double board_height = ed->getBoardDims(BOARDPROP::kHeight);
	double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
	auto board_type = ed->getBoardType();
	std::cout << "Looking for a calibration pattern " << board_width << " corners wide, " << board_height << " corners high\n";
	GenericProcessor * maybe_merger = getSourceNode();
	m_trackers.clear();
	bool showims = ed->saveCapturedImages();
	showCapturedImages(showims);
	if ( maybe_merger->isMerger() ) {
		for (int i = 0; i < 2; ++i) {
			maybe_merger->switchIO(i);
			PosTracker * tracker = (PosTracker*)maybe_merger->getSourceNode();
			if ( tracker != nullptr ) {
				tracker = (PosTracker*)maybe_merger->getSourceNode();
				tracker->openCamera();
				tracker->getEditor()->updateSettings();
				m_trackers.push_back(tracker);
				auto cal = std::make_unique<CalibrateCamera>(board_width, board_height, board_size, board_type);
				cal->setCameraName(tracker->getDevName());
				calibrators.push_back(std::move(cal));

			}
		}
	}
	m_threadRunning = true;
	startThread(); // calls run
}

void StereoPos::run() {
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	int pauseBetweenCapsSecs = ed->getNSecondsBetweenCaptures();
	bool showImages = ed->saveCapturedImages();
	cv::Mat frame;
	unsigned int count = 0;
	// containers for various parts of the opencv calibration algos
	std::vector<std::vector<cv::Mat>> images{m_trackers.size()};
	std::time_t starttime = std::time(nullptr);
	while ( ( count < (nImagesToCapture * m_trackers.size()) ) && m_threadRunning ) {
		std::time_t nowtime = std::time(nullptr);
		for (int i = 0; i < m_trackers.size(); ++i) {
			PosTracker * tracker = m_trackers[i];
			if ( tracker->isCamReady() ) {
				if ( std::difftime(nowtime, starttime) > pauseBetweenCapsSecs) {
					if ( tracker->isStreaming() ) {
						std::cout << "Capturing image on " << tracker->getDevName() << std::endl;
						Formats * currentFmt = tracker->getCurrentFormat();
						frame = cv::Mat(currentFmt->height, currentFmt->width, CV_8UC3, (unsigned char*)tracker->get_frame_ptr());
						cv::Mat frame_clone = frame.clone();
						images[i].push_back(frame_clone);
						tracker->stereoPosCapture(false);
						++count;
					}
				}
			}
		}
		std::time_t starttime = std::time(nullptr);
		sleep(pauseBetweenCapsSecs*1000);
	}
	for (int i = 0; i < m_trackers.size(); ++i) {
		std::cout << "Calibrating camera " << i << " with " << std::to_string(images[i].size()) << " images" << std::endl;
		calibrators[i]->setup(images[i], showImages);
		if ( calibrators[i]->calibrate() ) {
			auto cam_mat = calibrators[i]->getCameraMatrix();
			auto dist_coeffs = calibrators[i]->getDistCoeffs();
			std::cout << "cam_mat " << cam_mat << std::endl;
			std::cout << "dist_coeffs " << dist_coeffs << std::endl;
		}
	}

}

void StereoPos::stopStreaming() {
	if ( m_threadRunning ) {
		m_threadRunning = false;
		stopThread(1000);
		for (int i = 0; i < m_trackers.size(); ++i) {
			m_trackers[i]->stopCamera();
		}
	}
}

void StereoPos::saveCustomParametersToXml(XmlElement * xml)
{
	xml->setAttribute("Type", "StereoPos");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	paramXml->setAttribute("numImages", ed->getNImagesToCapture());
	paramXml->setAttribute("timeDelay", ed->getNSecondsBetweenCaptures());
	paramXml->setAttribute("numColumns", ed->getBoardDims(BOARDPROP::kWidth));
	paramXml->setAttribute("numRows", ed->getBoardDims(BOARDPROP::kHeight));
	paramXml->setAttribute("squareSize", ed->getBoardDims(BOARDPROP::kSquareSize));
}

void StereoPos::loadCustomParametersFromXml()
{
	auto ed = static_cast<StereoPosEditor*>(getEditor());
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{
		if ( paramXml->hasAttribute("numImages") )
			ed->setNImagesToCapture(paramXml->getIntAttribute("numImages"));
		if ( paramXml->hasAttribute("timeDelay") )
			ed->setNSecondsBetweenCaptures(paramXml->getIntAttribute("timeDelay"));
		if ( paramXml->hasAttribute("numColumns") )
			ed->setBoardDims(BOARDPROP::kWidth, paramXml->getIntAttribute("numColumns"));
		if ( paramXml->hasAttribute("numRows") )
			ed->setBoardDims(BOARDPROP::kHeight, paramXml->getIntAttribute("numRows"));
		if ( paramXml->hasAttribute("squareSize") )
			ed->setBoardDims(BOARDPROP::kSquareSize, paramXml->getIntAttribute("squareSize"));
	}
	updateSettings();
}
