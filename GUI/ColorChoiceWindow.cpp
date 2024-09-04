#include "ColorChoiceWindow.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QDialogButtonBox>

Color getColorByIndex(int i) {
	switch (i) {
		case 0: return Color::White;
		case 1: return Color::Black;
		case 2: return Color::Red;
		case 3: return Color::Green;
		case 4: return Color::Lime;
		case 5: return Color::Blue;
		case 6: return Color::Navy;
		case 7: return Color::Olive;
		case 8: return Color::Cyan;
		case 9: return Color::Gray;
		case 10: return Color::Silver;
		case 11: return Color::Orange;
		case 12: return Color::Yellow;
		case 13: return Color::Brown;
		case 14: return Color::Pink;
		case 15: return Color::Magenta;
	default: return Color::Black;
	}
}

ColorChoiceWindow::ColorChoiceWindow(QWidget* parent, Color color)
	: QDialog(parent) {
	setWindowTitle("Color choice menu");

	QGridLayout* gridLayout = new QGridLayout();
	for (int i = 0; i < 16; i++) {
		m_colorSetLabels[i] = new ColorLabel(nullptr, false, i);
		m_colorSetLabels[i]->setColor(getColorByIndex(i));
		
		connect(m_colorSetLabels[i], SIGNAL(clicked(int)), this, SLOT(colorSetClickedSlot(int)));

		gridLayout->addWidget(m_colorSetLabels[i], i / 4, i % 4);
	}

	m_rSlider = new EditSlider(nullptr, Qt::Horizontal, 0, 255, 1);
	m_gSlider = new EditSlider(nullptr, Qt::Horizontal, 0, 255, 1);
	m_bSlider = new EditSlider(nullptr, Qt::Horizontal, 0, 255, 1);
	m_aSlider = new EditSlider(nullptr, Qt::Horizontal, 0, 255, 1);

	m_rSlider->setValue(color.r());
	m_gSlider->setValue(color.g());
	m_bSlider->setValue(color.b());
	m_aSlider->setValue(color.a());

	connect(m_rSlider, SIGNAL(valueChanged(int)), this, SLOT(rChangedSlot(int)));
	connect(m_gSlider, SIGNAL(valueChanged(int)), this, SLOT(gChangedSlot(int)));
	connect(m_bSlider, SIGNAL(valueChanged(int)), this, SLOT(bChangedSlot(int)));
	connect(m_aSlider, SIGNAL(valueChanged(int)), this, SLOT(aChangedSlot(int)));

	auto getLabel = [&](QString str) -> QLabel* { // Needed to evade a bug with invisible labels
		QLabel* result = new QLabel(str);
		result->setStyleSheet("QLabel { color: black; background-color: rgba(0, 0, 0, 0%); border: 0px; }");
		return result;
	};

	QFormLayout* formLayout = new QFormLayout();
	formLayout->addRow(getLabel(tr("Red:")), m_rSlider);
	formLayout->addRow(getLabel(tr("Green:")), m_gSlider);
	formLayout->addRow(getLabel(tr("Blue:")), m_bSlider);
	formLayout->addRow(getLabel(tr("Alpha:")), m_aSlider);

	m_colorPreview = new ColorLabel();
	m_colorPreview->setColor(color);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout* boxLayout = new QVBoxLayout();
	boxLayout->addWidget(m_colorPreview, 1);
	boxLayout->addLayout(formLayout, 1);
	boxLayout->addLayout(gridLayout, 1);
	boxLayout->addWidget(buttonBox);

	setLayout(boxLayout);
}

Color ColorChoiceWindow::getChosenColor() {
	uint8_t r = m_rSlider->getValue();
	uint8_t g = m_gSlider->getValue();
	uint8_t b = m_bSlider->getValue();
	uint8_t a = m_aSlider->getValue();
	
	return Color(r, g, b, a);
}

bool ColorChoiceWindow::chooseColor(QWidget* parent, Color& color) {
	ColorChoiceWindow window(parent, color);

	if (window.exec() == QDialog::Accepted) {
		color = window.getChosenColor();
		return true;
	}

	return false;
}

void ColorChoiceWindow::rChangedSlot(int val) {
	Color color = m_colorPreview->getColor();
	color.r() = val;
	m_colorPreview->setColor(color);
}

void ColorChoiceWindow::gChangedSlot(int val) {
	Color color = m_colorPreview->getColor();
	color.g() = val;
	m_colorPreview->setColor(color);
}

void ColorChoiceWindow::bChangedSlot(int val) {
	Color color = m_colorPreview->getColor();
	color.b() = val;
	m_colorPreview->setColor(color);
}

void ColorChoiceWindow::aChangedSlot(int val) {
	Color color = m_colorPreview->getColor();
	color.a() = val;
	m_colorPreview->setColor(color);
}

void ColorChoiceWindow::colorSetClickedSlot(int id) {
	Color color = m_colorSetLabels[id]->getColor();
	m_colorPreview->setColor(color);

	m_rSlider->setValue(color.r());
	m_gSlider->setValue(color.g());
	m_bSlider->setValue(color.b());
	m_aSlider->setValue(color.a());
}
