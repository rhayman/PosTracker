#include "StereoPosEditor.h"
#include <unistd.h>

StereoPosEditor::StereoPosEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors=true)
	: GenericEditor(parentNode, useDefaultParameterEditors)
{
	int silksize;
	const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
	MemoryInputStream mis(silk, silksize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	Font font = Font(typeface);
	font.setHeight(9);

	desiredWidth = 425;
	m_proc = (StereoPos*)getProcessor();

	// add controls etc
	intervalText = std::make_unique<TextEditor>(String("Interval"));
	intervalText->setBounds(5, 25, 30, 20);
	intervalText->setText("1");
	intervalText->setTooltip("Interval between frame captures");
	addAndMakeVisible(intervalText.get());

	intervalLabel = std::make_unique<Label>("IntervalTime", "Interval(s)");
	intervalLabel->setBounds(40, 25, 90, 20);
	intervalLabel->setFont(font);
	intervalLabel->setEditable (false, false, false);
	intervalLabel->setJustificationType(Justification::centredLeft);
	intervalLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(intervalLabel.get());

	nImagesText = std::make_unique<TextEditor>(String("NumImages"));
	nImagesText->setBounds(5, 55, 30, 20);
	nImagesText->setText("5");
	nImagesText->setTooltip("The number of images to capture from each camera");
	addAndMakeVisible(nImagesText.get());

	nImagesLabel = std::make_unique<Label>("nImagesLabel", "# images");
	nImagesLabel->setBounds(40, 55, 80, 20);
	nImagesLabel->setFont(font);
	nImagesLabel->setEditable (false, false, false);
	nImagesLabel->setJustificationType(Justification::centredLeft);
	nImagesLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(nImagesLabel.get());

	captureButton = std::make_unique<UtilityButton>("Capture", Font ("Small Text", 10, Font::plain));
	captureButton->setBounds(5, 85, 80, 20);
	captureButton->setTooltip("testing various things");
	captureButton->addListener(this);
	addAndMakeVisible(captureButton.get());

	showVideoCapture = std::make_unique<ToggleButton>("Save captured images");
	showVideoCapture->setBounds(5, 105, 100, 20);
	showVideoCapture->setTooltip("Save the captured images");
	showVideoCapture->addListener(this);
	showVideoCapture->setEnabled(true);
	addAndMakeVisible(showVideoCapture.get());

	boardWidthText = std::make_unique<TextEditor>(String("Board width"));
	boardWidthText->setBounds(115, 25, 30, 20);
	boardWidthText->setTooltip("Width of the board");
	boardWidthText->setText(board_width);
	addAndMakeVisible(boardWidthText.get());

	boardWidthLabel = std::make_unique<Label>("boardWidthLabel", "Board corner rows");
	boardWidthLabel->setBounds(145, 25, 80, 20);
	boardWidthLabel->setFont(font);
	boardWidthLabel->setEditable (false, false, false);
	boardWidthLabel->setJustificationType(Justification::centredLeft);
	boardWidthLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardWidthLabel.get());

	boardHeightText = std::make_unique<TextEditor>(String("Board height"));
	boardHeightText->setBounds(115, 55, 30, 20);
	boardHeightText->setTooltip("Height of the board");
	boardHeightText->setText(board_height);
	addAndMakeVisible(boardHeightText.get());

	boardHeightLabel = std::make_unique<Label>("boardHeightLabel", "Board corner cols");
	boardHeightLabel->setBounds(145, 55, 80, 20);
	boardHeightLabel->setFont(font);
	boardHeightLabel->setEditable (false, false, false);
	boardHeightLabel->setJustificationType(Justification::centredLeft);
	boardHeightLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardHeightLabel.get());

	boardSquareSizeText = std::make_unique<TextEditor>(String("Square size"));
	boardSquareSizeText->setBounds(115, 85, 30, 20);
	boardSquareSizeText->setTooltip("The size of each square");
	boardSquareSizeText->setText(square_size);
	addAndMakeVisible(boardSquareSizeText.get());

	boardSquareSizeLabel = std::make_unique<Label>("boardSquareSizeLabel", "Square size (cm)");
	boardSquareSizeLabel->setBounds(145, 85, 80, 20);
	boardSquareSizeLabel->setFont(font);
	boardSquareSizeLabel->setEditable (false, false, false);
	boardSquareSizeLabel->setJustificationType(Justification::centredLeft);
	boardSquareSizeLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardSquareSizeLabel.get());

	calibrationPatternCombo = std::make_unique<ComboBox>("calibrationPattern");
	calibrationPatternCombo->setBounds(230, 25, 80, 20);
	calibrationPatternCombo->setTooltip("The calibration pattern");
	calibrationPatternCombo->setEditableText(false);
	calibrationPatternCombo->setJustificationType(Justification::centredLeft);
	calibrationPatternCombo->addItem("Chessboard", 1);
	calibrationPatternCombo->addItem("Circular (symmetric)", 2);
	calibrationPatternCombo->addItem("Circular (asymmetric)", 3);
	calibrationPatternCombo->setSelectedId(1, dontSendNotification);
	addAndMakeVisible(calibrationPatternCombo.get());

	calibrateButton = std::make_unique<UtilityButton>("Calibrate", Font ("Small Text", 10, Font::plain));
	calibrateButton->setBounds(230, 85, 80, 20);
	calibrateButton->setTooltip("Calibrate the two cameras");
	calibrateButton->addListener(this);
	addAndMakeVisible(calibrateButton.get());

}

int StereoPosEditor::getNSecondsBetweenCaptures() {
	return intervalText->getText().getIntValue();
}

void StereoPosEditor::setNSecondsBetweenCaptures(int secs) {
	intervalText->setText(String(secs));
}

int StereoPosEditor::getNImagesToCapture() {
	return nImagesText->getText().getIntValue();
}

void StereoPosEditor::setNImagesToCapture(int nIms) {
	nImagesText->setText(String(nIms));
}

void StereoPosEditor::buttonEvent(Button * button)
{
	if ( button == captureButton.get() ) {
		m_proc->startStreaming();
	}
	if ( button == showVideoCapture.get() ) {
		m_saveCapture = showVideoCapture->getToggleState();
		m_proc->showCapturedImages(m_saveCapture);
	}
	if ( button == calibrateButton.get() ) {
		m_proc->doStereoCalibration();
	}
}

double StereoPosEditor::getBoardDims(BOARDPROP prop) {
	switch (prop) {
		case BOARDPROP::kWidth: {
			return boardWidthText->getText().getDoubleValue();
		}
		case BOARDPROP::kHeight: {
			return boardHeightText->getText().getDoubleValue();
		}
		case BOARDPROP::kSquareSize: {
			return boardSquareSizeText->getText().getDoubleValue();
		}
	}
}

void StereoPosEditor::setBoardDims(BOARDPROP prop, int val) {
	switch (prop) {
		case BOARDPROP::kWidth: {
			boardWidthText->setText(String(val));
			break;
		}
		case BOARDPROP::kHeight: {
			boardHeightText->setText(String(val));
			break;
		}
		case BOARDPROP::kSquareSize: {
			return boardSquareSizeText->setText(String(val));
			break;
		}
	}
}

BOARDPROP StereoPosEditor::getBoardType() {
	int board_type = calibrationPatternCombo->getSelectedId();
	switch (board_type) {
		case 1:
			return BOARDPROP::kChessBoard;
		case 2:
			return BOARDPROP::kCircularSymmetric;
		case 3:
			return BOARDPROP::kCircularAsymmetric;
	}
}

void StereoPosEditor::setBoardType(int val) {
	calibrationPatternCombo->setSelectedId(val, dontSendNotification);
}