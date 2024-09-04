#include "LineWidget.h"

QFrame* createVerticalLine(int width, QString style) {
    QFrame* lineWidget = new QFrame();
    lineWidget->setFrameShape(QFrame::VLine);
    lineWidget->setFrameShadow(QFrame::Plain);
    lineWidget->setLineWidth(width);
    lineWidget->setStyleSheet(style);

    return lineWidget;
}
