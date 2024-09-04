#include "ImageWindow.h"
#include <QBoxLayout>
#include <QLabel>
#include <QResizeEvent>

ImageWindow::ImageWindow(QWidget* parent, const QList<QPair<QString, QString>>& info)
	: QDialog(parent) {
    setWindowTitle(tr("Image information"));
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    setStyleSheet("background-color: rgb(253, 109, 13); color: rgb(255, 239, 204);");

    QList<QLabel*> labels;
    for (auto& pair : info) {
        labels.push_back(new QLabel("<b>" + pair.first + "</b>: " + pair.second, this));
    }

    QVBoxLayout* boxLayout = new QVBoxLayout(this);
    for (auto label : labels) {
        boxLayout->addWidget(label);
    }

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}
