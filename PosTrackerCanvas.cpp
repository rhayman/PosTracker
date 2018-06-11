#include "PosTrackerCanvas.h"
#include "PosTrackerEditor.h"

double WIDTH_PRC = 0.75;
double HEIGHT_PRC = 0.75;
int count = 0;
PosTrackerCanvas::PosTrackerCanvas(PosTracker * parentNode) : processor(parentNode)
{
    int silksize;
    const char* silk = CoreServices::getApplicationResource("silkscreenserialized", silksize);
    MemoryInputStream mis(silk, silksize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    Font font = Font(typeface);

	displayView = new Viewport();
	addAndMakeVisible(displayView);
    
    addAndMakeVisible(paramContainer = new GroupComponent("Parameters", "Parameters"));
    paramContainer->setBounds(180, 10, 300, 300);

    addAndMakeVisible(trackerType = new ComboBox("Tracker"));
    trackerType->setBounds(10, 10, 160, 20);
    trackerType->setTooltip("Tracker algorithm");
    trackerType->setTextWhenNothingSelected (("Resolution/ fps"));
    trackerType->setTextWhenNoChoicesAvailable (("Resolution / fps"));
    trackerType->setJustificationType (Justification::centredLeft);
    trackerType->setEditableText (false);
    for (int i = 0; i < trackerTypes.size(); ++i)
        trackerType->addItem(trackerTypes[i], i+1);
    trackerType->setSelectedId(1, sendNotificationSync);
    trackerType->addListener(this);
    
    // makeBoostingPanel();

    // editor = (PosTrackerEditor*)processor->getEditor();
    // setSize(400,400);

}

void PosTrackerCanvas::comboBoxChanged(ComboBox* cb) {
    int selected = trackerType->getSelectedId();
    if ( selected == 1 ) { // Boosting
        std::cout << "making boosting " << std::endl;
        makeBoostingPanel();
    }
    else if ( selected == 2 ) { //KCF
        std::cout << "making KCF " << std::endl;
        makeKCFPanel();
    }
    else if ( selected == 3 ) { // MedianFlow

    }
    else if ( selected == 4 ) { // MIL

    }
}

void PosTrackerCanvas::makeBoostingPanel() {
    
    addAndMakeVisible(boosty = new BoostingComponent());
    addSliderToGroup(paramContainer, 0.0, 20.0, 1.0);
    // paramContainer->repaint();
}

ScopedPointer<Slider> PosTrackerCanvas::addSliderToGroup(ScopedPointer<GroupComponent> group, double min, double max, double step) {
    sb = new Slider("Slider");
    // group->addAndMakeVisible(sb);
    sb->setRange(min, max, step);
    sb->setValue(3.0);
    sb->addListener(this);
    return sb;
}

void PosTrackerCanvas::addLabelToGroup() {
    addAndMakeVisible(leftRightLbl = new Label("leftRightLbl", "Left-Right"));
    leftRightLbl->setBounds(130, 80, 70, 20);
    leftRightLbl->setEditable (false, false, false);
    leftRightLbl->setJustificationType(Justification::centred);
    leftRightLbl->setColour (TextEditor::textColourId, Colours::grey);
    paramContainer->addAndMakeVisible(leftRightLbl);
    refresh();
}

void PosTrackerCanvas::makeKCFPanel() {
    // ScopedPointer<Slider> sb = addSliderToGroup(paramContainer, 0.0, 20.0, 1.0);
}

void PosTrackerCanvas::paint(Graphics & g)
{
    g.fillAll(Colours::darkgrey);
}

void PosTrackerCanvas::refresh()
{
	repaint();
}

void PosTrackerCanvas::beginAnimation()
{
	startCallbacks();
}

void PosTrackerCanvas::endAnimation()
{
	stopCallbacks();
}

void PosTrackerCanvas::refreshState()
{
	update();
	resized();
}

void PosTrackerCanvas::resized()
{
    

}

void PosTrackerCanvas::update()
{
    
}

void PosTrackerCanvas::labelTextChanged(Label * labelChanged)
{}

void PosTrackerCanvas::sliderValueChanged(Slider * sliderChanged)
{
    
}

void PosTrackerCanvas::buttonClicked(Button * button)
{
}
