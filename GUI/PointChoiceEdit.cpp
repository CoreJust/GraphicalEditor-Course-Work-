#include "PointChoiceEdit.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QValidator>
#include <QIcon>

// QIcon g_choosePointIcon = QIcon("res/icons/choose_point_icon.png");

PointChoiceEdit::PointChoiceEdit(QWidget* parent, uint32_t maxX, uint32_t maxY)
	: QWidget(parent) {
	m_xEdit = new QLineEdit();
	m_xEdit->setValidator(new QIntValidator(0, maxX, this));

	m_yEdit = new QLineEdit();
	m_yEdit->setValidator(new QIntValidator(0, maxY, this));

	QFormLayout* formLayout = new QFormLayout();
	formLayout->addRow("X: ", m_xEdit);
	formLayout->addRow("Y: ", m_yEdit);

	setLayout(formLayout);
}

QPoint PointChoiceEdit::getChosenPoint() {
	return QPoint(m_xEdit->text().toInt(), m_yEdit->text().toInt());
}

bool PointChoiceEdit::isChosen() {
	return !m_xEdit->text().isEmpty() && !m_yEdit->text().isEmpty();
}
