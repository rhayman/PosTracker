#include "StereoPosEditor.h"

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
	intervalText->setBounds(5, 5, 80, 20);
	intervalText->setTooltip("Interval between frame captures");
	intervalText->addListener(this);
	
	intervalLabel = new Label("IntervalTime", "Interval (s)");
	intervalLabel->setBounds(90, 5, 80, 20);
	intervalLabel->setFont(font);
	intervalLabel->setEditable (false, false, false);
	intervalLabel->setJustificationType(Justification::centredLeft);
	intervalLabel->setColour (TextEditor::textColourId, Colours::grey);
	addAndMakeVisible(intervalLabel);

	addAndMakeVisible(captureButton = new UtilityButton("Capture", Font ("Small Text", 10, Font::plain)));
	captureButton->setBounds(5, 35, 80, 20);
	captureButton->setTooltip("testing various things");
	captureButton->addListener(this);


}

void StereoPosEditor::buttonEvent(Button * button)
{
	if ( button == captureButton ) {
		m_proc->testFcn();
	}
}