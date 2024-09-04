#pragma once
#include <QDialog>
#include <QFormLayout>
#include <QCheckBox>
#include "EditSlider.h"
#include "FilePathEdit.h"
#include "ColorLabel.h"
#include "PointChoiceEdit.h"

// A dialog setting the area of use of the tool, its color, etc
// List of possible fields: LineEdit, EditSlider, ColorLabel, PointChoiceEdit, FilePathEdit, CheckBox, Label
class CustomInputDialog final : public QDialog {
private:
	QList<QCheckBox*> m_checkBoxes;
	QList<QLineEdit*> m_lineEdits;
	QList<QLabel*> m_labels;
	QList<EditSlider*> m_editSliders;
	QList<FilePathEdit*> m_filePathEdits;
	QList<PointChoiceEdit*> m_pointChoiceEdits;
	QList<ColorLabel*> m_colorLabels;
	QFormLayout* m_layout = nullptr;

public:
	CustomInputDialog(QWidget* parent = nullptr, const QString& title = "");

	// Adders return the id of the widget which can be used to get its value later

	int addCheckBox(const QString& label, QCheckBox* widget);
	bool getCheckBoxValue(int id);

	int addLineEdit(const QString& label, QLineEdit* widget);
	QString getLineEditValue(int id);

	void addLabel(QLabel* widget);

	int addEditSlider(const QString& label, EditSlider* widget);
	int getEditSliderValue(int id);

	int addFilePathEdit(const QString& label, FilePathEdit* widget);
	QString getFilePathEditValue(int id);

	int addPointChoiceEdit(const QString& label, PointChoiceEdit* widget);
	QPoint getPointChoiceEditValue(int id);

	int addColorLabel(const QString& label, ColorLabel* widget);
	Color getColorLabelValue(int id);

	// Must be called after all the widget were added to the dialog
	void finishSetupUI();

private slots: // So that it can only be accepted when all the necessary fields are filled in (line edits and point edits)
	void accept() override;
};