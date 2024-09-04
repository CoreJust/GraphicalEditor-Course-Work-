#include "CustomInputDialog.h"
#include <QDialogButtonBox>
#include <QMessageBox>

CustomInputDialog::CustomInputDialog(QWidget* parent, const QString& title)
	: QDialog(parent) {
	setWindowTitle(title);

	m_layout = new QFormLayout();
}

int CustomInputDialog::addCheckBox(const QString& label, QCheckBox* widget) {
	m_checkBoxes.push_back(widget);
	m_layout->addRow(label, widget);

	return m_checkBoxes.size() - 1;
}

bool CustomInputDialog::getCheckBoxValue(int id) {
	assert(id < m_checkBoxes.size());

	return m_checkBoxes[id]->isChecked();
}

int CustomInputDialog::addLineEdit(const QString& label, QLineEdit* widget) {
	m_lineEdits.push_back(widget);
	m_layout->addRow(label, widget);

	return m_lineEdits.size() - 1;
}

QString CustomInputDialog::getLineEditValue(int id) {
	assert(id < m_lineEdits.size());

	return m_lineEdits[id]->text();
}

void CustomInputDialog::addLabel(QLabel* widget) {
	m_labels.push_back(widget);
	m_layout->addWidget(widget);
}

int CustomInputDialog::addEditSlider(const QString& label, EditSlider* widget) {
	m_editSliders.push_back(widget);
	m_layout->addRow(label, widget);

	return m_editSliders.size() - 1;
}

int CustomInputDialog::getEditSliderValue(int id) {
	assert(id < m_editSliders.size());

	return m_editSliders[id]->getValue();
}

int CustomInputDialog::addFilePathEdit(const QString& label, FilePathEdit* widget) {
	m_filePathEdits.push_back(widget);
	m_layout->addRow(label, widget);

	return m_filePathEdits.size() - 1;
}

QString CustomInputDialog::getFilePathEditValue(int id) {
	assert(id < m_filePathEdits.size());

	return m_filePathEdits[id]->getPath();
}

int CustomInputDialog::addPointChoiceEdit(const QString& label, PointChoiceEdit* widget) {
	m_pointChoiceEdits.push_back(widget);
	m_layout->addRow(label, widget);

	return m_pointChoiceEdits.size() - 1;
}

QPoint CustomInputDialog::getPointChoiceEditValue(int id) {
	assert(id < m_pointChoiceEdits.size());

	return m_pointChoiceEdits[id]->getChosenPoint();
}

int CustomInputDialog::addColorLabel(const QString& label, ColorLabel* widget) {
	m_colorLabels.push_back(widget);
	m_layout->addRow(label, widget);

	return m_colorLabels.size() - 1;
}

Color CustomInputDialog::getColorLabelValue(int id) {
	assert(id < m_colorLabels.size());

	return m_colorLabels[id]->getColor();
}

void CustomInputDialog::finishSetupUI() {
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	m_layout->addWidget(buttonBox);

	setLayout(m_layout);
}

void CustomInputDialog::accept() {
	auto printErrorMessage = []() {
		QMessageBox* msg = new QMessageBox();

		msg->setWindowTitle("Warning");
		msg->setText("Not all the necessary fields were filled in!");
		msg->show();
	};

	for (auto line : m_lineEdits) {
		if (line->text().isEmpty()) {
			printErrorMessage();
			return;
		}
	}

	for (auto point : m_pointChoiceEdits) {
		if (!point->isChosen()) {
			printErrorMessage();
			return;
		}
	}

	QDialog::accept();
}