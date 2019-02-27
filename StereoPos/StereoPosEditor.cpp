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
	addAndMakeVisible(intervalText = new TextEditor(String("Interval")));
	intervalText->setBounds(5, 25, 30, 20);
	intervalText->setText("1");
	intervalText->setTooltip("Interval between frame captures");

	intervalLabel = new Label("IntervalTime", "Interval(s)");
	intervalLabel->setBounds(40, 25, 90, 20);
	intervalLabel->setFont(font);
	intervalLabel->setEditable (false, false, false);
	intervalLabel->setJustificationType(Justification::centredLeft);
	intervalLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(intervalLabel);

	addAndMakeVisible(nImagesText = new TextEditor(String("NumImages")));
	nImagesText->setBounds(5, 55, 30, 20);
	nImagesText->setText("5");
	nImagesText->setTooltip("The number of images to capture from each camera");

	nImagesLabel = new Label("nImagesLabel", "# images");
	nImagesLabel->setBounds(40, 55, 80, 20);
	nImagesLabel->setFont(font);
	nImagesLabel->setEditable (false, false, false);
	nImagesLabel->setJustificationType(Justification::centredLeft);
	nImagesLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(nImagesLabel);

	addAndMakeVisible(captureButton = new UtilityButton("Capture", Font ("Small Text", 10, Font::plain)));
	captureButton->setBounds(5, 85, 80, 20);
	captureButton->setTooltip("testing various things");
	captureButton->addListener(this);

	addAndMakeVisible(showVideoCapture = new ToggleButton("Show capture"));
	showVideoCapture->setBounds(5, 105, 100, 20);
	showVideoCapture->setTooltip("Show the captured images");
	showVideoCapture->addListener(this);
	showVideoCapture->setEnabled(true);

	addAndMakeVisible(boardWidthText = new TextEditor(String("Board width")));
	boardWidthText->setBounds(115, 25, 30, 20);
	boardWidthText->setTooltip("Width of the board");
	boardWidthText->setText(board_width);

	boardWidthLabel = new Label("boardWidthLabel", "Board corner rows");
	boardWidthLabel->setBounds(145, 25, 80, 20);
	boardWidthLabel->setFont(font);
	boardWidthLabel->setEditable (false, false, false);
	boardWidthLabel->setJustificationType(Justification::centredLeft);
	boardWidthLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardWidthLabel);

	addAndMakeVisible(boardHeightText = new TextEditor(String("Board height")));
	boardHeightText->setBounds(115, 55, 30, 20);
	boardHeightText->setTooltip("Height of the board");
	boardHeightText->setText(board_height);

	boardHeightLabel = new Label("boardHeightLabel", "Board corner cols");
	boardHeightLabel->setBounds(145, 55, 80, 20);
	boardHeightLabel->setFont(font);
	boardHeightLabel->setEditable (false, false, false);
	boardHeightLabel->setJustificationType(Justification::centredLeft);
	boardHeightLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardHeightLabel);

	addAndMakeVisible(boardSquareSizeText = new TextEditor(String("Square size")));
	boardSquareSizeText->setBounds(115, 85, 30, 20);
	boardSquareSizeText->setTooltip("The size of each square");
	boardSquareSizeText->setText(square_size);

	boardSquareSizeLabel = new Label("boardSquareSizeLabel", "Square size (cm)");
	boardSquareSizeLabel->setBounds(145, 85, 80, 20);
	boardSquareSizeLabel->setFont(font);
	boardSquareSizeLabel->setEditable (false, false, false);
	boardSquareSizeLabel->setJustificationType(Justification::centredLeft);
	boardSquareSizeLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(boardSquareSizeLabel);
}

int StereoPosEditor::getNSecondsBetweenCaptures() {
	return intervalText->getText().getIntValue();
}

int StereoPosEditor::getNImagesToCapture() {
	return nImagesText->getText().getIntValue();
}


void StereoPosEditor::buttonEvent(Button * button)
{
	if ( button == captureButton ) {
		m_proc->startStreaming();
	}
	if ( button == showVideoCapture ) {
		m_setShowCapture = showVideoCapture->getToggleState();
		m_proc->showCapturedImages(m_setShowCapture);
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