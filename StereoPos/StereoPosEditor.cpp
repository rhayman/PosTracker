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

	desiredWidth = 425;
	m_proc = (StereoPos*)getProcessor();

	// add controls etc
	addAndMakeVisible(intervalText = new TextEditor(String("Interval")));
	intervalText->setBounds(5, 25, 80, 20);
	intervalText->setTooltip("Interval between frame captures");

	intervalLabel = new Label("IntervalTime", "Interval(s)");
	intervalLabel->setBounds(90, 25, 90, 20);
	intervalLabel->setFont(font);
	intervalLabel->setEditable (false, false, false);
	intervalLabel->setJustificationType(Justification::centredLeft);
	intervalLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(intervalLabel);

	addAndMakeVisible(captureButton = new UtilityButton("Capture", Font ("Small Text", 10, Font::plain)));
	captureButton->setBounds(5, 55, 80, 20);
	captureButton->setTooltip("testing various things");
	captureButton->addListener(this);

	addAndMakeVisible(showVideoCapture = new ToggleButton("Show capture"));
	showVideoCapture->setBounds(5, 105, 100, 20);
	showVideoCapture->setTooltip("Show the captured images");
	showVideoCapture->addListener(this);
	showVideoCapture->setEnabled(true);

	addAndMakeVisible(boardWidthText = new TextEditor(String("Board width")));
	boardWidthText->setBounds(165, 25, 80, 20);
	boardWidthText->setTooltip("Width of the board");
	boardWidthText->setText(board_width);

	addAndMakeVisible(boardHeightText = new TextEditor(String("Board height")));
	boardHeightText->setBounds(165, 55, 80, 20);
	boardHeightText->setTooltip("Height of the board");
	boardHeightText->setText(board_height);

	addAndMakeVisible(boardSquareSizeText = new TextEditor(String("Square size")));
	boardSquareSizeText->setBounds(165, 85, 80, 20);
	boardSquareSizeText->setTooltip("The size of each square");
	boardSquareSizeText->setText(square_size);
}

void StereoPosEditor::buttonEvent(Button * button)
{
	if ( button == captureButton ) {
		String time = intervalText->getText();
		int interval = time.getIntValue();
		if ( interval <= 0 )
			interval = 1;
		std::cout << "interval " << interval << std::endl;
		unsigned int count = 0;
		while (count < nCalibrationImagesToCaputure) {
			std::cout << count << std::endl;
			sleep(interval);
			++count;
		}
		m_proc->testFcn();
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