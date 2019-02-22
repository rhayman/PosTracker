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
	addAndMakeVisible(testButton = new ToggleButton("Test button"));
	testButton->setBounds(5, 105, 100, 20);
	testButton->setTooltip("testing various things");
	testButton->addListener(this);
	testButton->setEnabled(true);

}

void StereoPosEditor::buttonEvent(Button * button)
{
	if ( button == testButton ) {
		
			m_proc->testFcn();
		}
}