#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>
#include "StereoPos.h"
#include "StereoPosEditor.h"
#include "../PosTracker/PosTracker.h"
#include "../PosTracker/Camera.h"

class CalibrateCamera {
public:
	CalibrateCamera() {};
	CalibrateCamera(double width, double height, double size) : m_width(width), m_height(height), m_size(size) {};
	~CalibrateCamera() {};
	void setup(std::vector<cv::Mat> imgs, bool showImages=false) {
		cv::Size board_size = cv::Size(m_width, m_height);
		std::vector<cv::Point2f> corners;
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			std::cout << "greyimgs[i].size() " << imgs[i].size() << std::endl;
			if ( showImages ) {
				cv::imshow("grey", grey);
				cv::waitKey(1);
			}
			bool found = false;
			found = cv::findChessboardCorners(imgs[i], board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
			if ( found ) {
				std::cout << "Got a chess board" << std::endl;
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				cv::drawChessboardCorners(grey, board_size, corners, found);
			}
		}
	};
private:
	double m_width = 137;
	double m_height = 148;
	double m_size = 11;
};

StereoPos::StereoPos() : GenericProcessor("Stereo Pos")
{
	setProcessorType (PROCESSOR_TYPE_FILTER);
	sendSampleCount = false;
	// calibrator = std::make_unique<CalibrateCamera>();
}

StereoPos::~StereoPos() {
	 calibrator.reset();
}

void StereoPos::process(AudioSampleBuffer& buffer)
{}

void StereoPos::updateSettings()
{

}

AudioProcessorEditor * StereoPos::createEditor() {
	editor = new StereoPosEditor(this, true);
	return editor;

}

void StereoPos::showCapturedImages(bool show) {
	if ( show )
		cv::namedWindow("grey", cv::WINDOW_NORMAL & cv::WND_PROP_ASPECT_RATIO & cv::WINDOW_GUI_NORMAL);
	else
		cv::destroyWindow("grey");

}

void StereoPos::testFcn() {
	GenericProcessor * maybe_merger = getSourceNode();
	if ( maybe_merger->isMerger() ) {
		auto ed = static_cast<StereoPosEditor*>(getEditor());
		bool showImages = ed->showCapturedImages();
		double board_width = ed->getBoardDims(BOARDPROP::kWidth);
		double board_height = ed->getBoardDims(BOARDPROP::kHeight);
		double board_size = ed->getBoardDims(BOARDPROP::kSquareSize);
		maybe_merger->switchIO(0); // sourceNodeA
		PosTracker* video_A = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_A ) {
			std::shared_ptr<Camera> thiscam = video_A->getCurrentCamera();
			video_A->openCamera();
			sleep(1);
			if ( video_A->isCamReady() ) {
				video_A->getEditor()->updateSettings();
				cv::Mat img;
				struct timeval tv;
				thiscam->read_frame(img, tv);
				if ( ! img.empty() ) {
					std::cout << "Calibrating " << video_A->getDeviceName() << "..." << std::endl;
					std::vector<cv::Mat> ims;
					ims.push_back(img);
					calibrator = std::make_unique<CalibrateCamera>(board_width, board_height, board_size);
					calibrator->setup(ims, showImages);
				}
			}
		}
		maybe_merger->switchIO(1); // sourceNodeA
		PosTracker* video_B = (PosTracker*)maybe_merger->getSourceNode();
		if ( video_B ) {
			std::cout << "video_B->getDeviceName() " << video_B->getDeviceName() << std::endl;
		}
	}
}

void StereoPos::saveCustomParametersToXml(XmlElement * xml)
{
	xml->setAttribute("Type", "StereoPos");
	XmlElement * paramXml = xml->createNewChildElement("Parameters");
}

void StereoPos::loadCustomParametersFromXml()
{
	forEachXmlChildElementWithTagName(*parametersAsXml, paramXml, "Parameters")
	{

	}
	updateSettings();
}
