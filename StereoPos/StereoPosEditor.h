#ifndef STEREOPOSEDITOR_H_
#define STEREOPOSEDITOR_H_

#include <memory>
#include <EditorHeaders.h>
#include "StereoPos.h"

enum class BOARDPROP : int {
	kChessBoard = 1,
	kCircularSymmetric,
	kCircularAsymmetric,
	kWidth,
	kHeight,
	kSquareSize
};

class StereoPosEditor : public GenericEditor
{
public:
	StereoPosEditor(GenericProcessor * parentNode, bool useDefaultParameterEditors);
	~StereoPosEditor() {};

	void editorShown(Label*, TextEditor& textEditorShown) {};

	void buttonEvent(Button* button);

	void updateSettings() {};
	void update() {};

	double getBoardDims(BOARDPROP);
	void setBoardDims(BOARDPROP, int);
	int getNImagesToCapture();
	void setNImagesToCapture(int);
	int getNSecondsBetweenCaptures();
	void setNSecondsBetweenCaptures(int);
	BOARDPROP getBoardType();
	void setBoardType(int);

	void saveCapturedImages(bool val) { m_saveCapture = val; }
	bool saveCapturedImages() { return m_saveCapture; }

private:
	StereoPos * m_proc;

	std::unique_ptr<TextEditor> intervalText;
	std::unique_ptr<TextEditor> nImagesText;
	std::unique_ptr<TextEditor> boardWidthText;
	std::unique_ptr<TextEditor> boardHeightText;
	std::unique_ptr<TextEditor> boardSquareSizeText;
	std::unique_ptr<ComboBox> calibrationPatternCombo;
	std::unique_ptr<Label> intervalLabel;
	std::unique_ptr<Label> nImagesLabel;
	std::unique_ptr<Label> boardWidthLabel;
	std::unique_ptr<Label> boardHeightLabel;
	std::unique_ptr<Label> boardSquareSizeLabel;
	std::unique_ptr<Label> calibrationPatternLabel;
	std::unique_ptr<UtilityButton> captureButton;
	std::unique_ptr<UtilityButton> calibrateButton;
	std::unique_ptr<ToggleButton> showVideoCapture;

	bool m_saveCapture = false;

	String board_width = String("11");
	String board_height = String("12");
	String square_size = String("11");

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoPosEditor);
};

#endif