#include "EditSlider.h"
#include <QValidator>

EditSlider::EditSlider(QWidget* parent, Qt::Orientation orientation, int valMin, int valMax, int step)
	: QWidget(parent) {
	m_lineEdit = new QLineEdit();
	m_lineEdit->setValidator(new QIntValidator(valMin, valMax, this));

	connect(m_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(lineEditValueChangedSlot(const QString&)));

	m_slider = new QSlider(orientation);
	m_slider->setMinimum(valMin);
	m_slider->setMaximum(valMax);
	m_slider->setSingleStep(step);

	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChangedSlot(int)));

	m_layout = new QVBoxLayout();

	if (orientation == Qt::Horizontal) {
		m_layout->addWidget(m_lineEdit);
		m_layout->addWidget(m_slider);
	} else { // Vertical
		m_layout->addWidget(m_slider);
		m_layout->addWidget(m_lineEdit);
	}

	setLayout(m_layout);
}

void EditSlider::setValue(int value) {
	m_slider->setValue(value);
	m_lineEdit->setText(QString::number(value));
}

int EditSlider::getValue() {
	return m_slider->value();
}

void EditSlider::lineEditValueChangedSlot(const QString& newVal) {
	int val = newVal.toInt();
	m_slider->setValue(val);

	emit valueChanged(val);
}

void EditSlider::sliderValueChangedSlot(int newVal) {
	newVal = newVal;
	m_lineEdit->setText(QString::number(newVal));

	emit valueChanged(newVal);
}
