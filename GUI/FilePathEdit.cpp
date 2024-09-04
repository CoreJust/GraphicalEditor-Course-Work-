#include "FilePathEdit.h"
#include <QBoxLayout>
#include <QIcon>
#include <QFileDialog>

FilePathEdit::FilePathEdit(QWidget* parent, const QString& defaultPath, const QString& fileTypes, bool isDirectoryMode)
    : QWidget(parent), m_fileTypes(fileTypes), m_isDirectoryMode(isDirectoryMode) {
    m_lineEdit = new QLineEdit();
    m_lineEdit->setText(defaultPath.isEmpty() ? QDir::currentPath() : defaultPath);

    m_fileDialogButton = new QPushButton();
    m_fileDialogButton->setIcon(QIcon("res/icon/file_icon.png"));
    connect(m_fileDialogButton, SIGNAL(clicked()), this, SLOT(fileDialogButtonClickedSlot()));

    QHBoxLayout* boxLayout = new QHBoxLayout();
    boxLayout->addWidget(m_lineEdit);
    boxLayout->addWidget(m_fileDialogButton);

    setLayout(boxLayout);
}

QString FilePathEdit::getPath() {
    return m_lineEdit->text();
}

void FilePathEdit::fileDialogButtonClickedSlot() {
    QString fileName;

    if (!m_isDirectoryMode) {
        fileName = QFileDialog::getSaveFileName(
            this,
            tr("Select the file"),
            getPath(),
            m_fileTypes
        );
    } else {
        fileName = QFileDialog::getExistingDirectory(
            this,
            tr("Select the directory"),
            getPath()
        );
    }

    if (!fileName.isEmpty()) {
        m_lineEdit->setText(fileName);
    }
}
