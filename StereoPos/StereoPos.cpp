#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>
#include "StereoPos.h"
#include "StereoPosEditor.h"
#include "../PosTracker/PosTracker.h"
#include "../PosTracker/Camera.h"

const double board_width = 13.7;
const double board_height = 14.8;
const double square_size = 1.1;

class CalibrateCamera {
public:
	CalibrateCamera() {};
	~CalibrateCamera() {};
	void setup(std::vector<cv::Mat> imgs, bool showImages=false) {
		cv::Size board_size = cv::Size(board_width, board_height);
		std::vector<cv::Point2f> corners;
		for (int i = 0; i < imgs.size(); ++i)
		{
			cv::Mat grey;
			cv::cvtColor(imgs[i], grey, cv::COLOR_BGR2GRAY);
			std::cout << "greyimgs[i].size() " << imgs[i].size() << std::endl;
			if ( showImages ) {
				cv::namedWindow("grey", cv::WINDOW_NORMAL & cv::WND_PROP_ASPECT_RATIO & cv::WINDOW_GUI_NORMAL);
				cv::imshow("grey", grey);
				cv::waitKey(1);
			}
			else
				cv::destroyWindow("grey");
			bool found = false;
			found = cv::findChessboardCorners(imgs[i], board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
			if ( found ) {
				std::cout << "Got a chess board" << std::endl;
				cv::cornerSubPix(grey, corners, cv::Size(5,5), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1));
				cv::drawChessboardCorners(grey, board_size, corners, found);
			}
		}
	};
// private:

};

StereoPos::StereoPos() : GenericProcessor("Stereo Pos")
{
	setProcessorType (PROCESSOR_TYPE_FILTER);
	sendSampleCount = false;
	calibrator = std::make_unique<CalibrateCamera>();
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

void StereoPos::testFcn() {
	GenericProcessor * maybe_merger = getSourceNode();
	if ( maybe_merger->isMerger() ) {
		auto ed = static_cast<StereoPosEditor*>(getEditor());
		bool showImages = ed->showCapturedImages();
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
				std::cout << "Calibrating " << video_A->getDeviceName() << "..." << std::endl;
				std::vector<cv::Mat> ims;
				ims.push_back(img);
				
				calibrator->setup(ims, showImages);
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
