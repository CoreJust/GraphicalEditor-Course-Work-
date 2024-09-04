#include "CW2_GraphicalEditor.h"
#include <QtWidgets/QFileDialog>
#include <QBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QValidator>
#include <QDialogButtonBox>
#include <QLabel>
#include <QEventLoop>
#include <QCloseEvent>
#include "../Utils/ScopeExit.h"
#include "ImageWindow.h"
#include "HelpWindow.h"
#include "ColorChoiceWindow.h"
#include "ProgressDialog.h"
#include "LineWidget.h"
#include "CustomInputDialog.h"
#include "../Script/Compiler.h"


constexpr uint32_t MINIMUM_WINDOW_WIDTH = 400;
constexpr uint32_t MINIMUM_WINDOW_HEIGHT = 300;
constexpr uint32_t BASE_WINDOW_WIDTH = 800;
constexpr uint32_t BASE_WINDOW_HEIGHT = 600;

const char* BASIC_TITLE = "Graphical editor cw2 by Yegor Ilyin";

CW2_GraphicalEditor::CW2_GraphicalEditor(QWidget *parent)
    : QMainWindow(parent) {
    // Setuping the main window
    setWindowTitle(tr(BASIC_TITLE));
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    setMinimumSize(MINIMUM_WINDOW_WIDTH, MINIMUM_WINDOW_HEIGHT);
    resize(BASE_WINDOW_WIDTH, BASE_WINDOW_HEIGHT);

    loadIcons();
    setupUI();
}

CW2_GraphicalEditor::~CW2_GraphicalEditor() {

}

void CW2_GraphicalEditor::loadIcons() {
    m_movingToolIcon = QIcon("res/icon/moving_tool.png");
    m_penToolIcon = QIcon("res/icon/pen_tool.png");
    m_pickToolIcon = QIcon("res/icon/pick_tool.png");
}

void CW2_GraphicalEditor::setupUI() {
    // Actions
    // File actions
    m_newFileAction = new QAction(tr("New"), this);
    m_newFileAction->setShortcut(QKeySequence::New);
    m_newFileAction->setStatusTip(tr("Create a new image"));
    connect(m_newFileAction, SIGNAL(triggered()), this, SLOT(newFileSlot()));


    m_openFileAction = new QAction(tr("Open"), this);
    m_openFileAction->setStatusTip("Open an image file");
    connect(m_openFileAction, SIGNAL(triggered()), this, SLOT(openFileSlot()));


    m_saveFileAction = new QAction(tr("Save"), this);
    m_saveFileAction->setShortcut(QKeySequence::Save);
    m_saveFileAction->setStatusTip(tr("Save the image"));
    connect(m_saveFileAction, SIGNAL(triggered()), this, SLOT(saveFileSlot()));


    m_saveAsFileAction = new QAction(tr("Save as"), this);
    m_saveAsFileAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsFileAction->setStatusTip(tr("Save the image to a chosen file"));
    connect(m_saveAsFileAction, SIGNAL(triggered()), this, SLOT(saveAsFileSlot()));


    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the program"));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(exitFileSlot()));


    // Tool actions
    m_componentFilterAction = new QAction(tr("Filter by channel"), this);
    m_componentFilterAction->setStatusTip(tr("Sets the value of some color channel to a fixed value"));
    connect(m_componentFilterAction, SIGNAL(triggered()), this, SLOT(filterToolSlot()));


    m_imageSplitAction = new QAction(tr("Split image into subimages"), this);
    m_imageSplitAction->setStatusTip(tr("Allows to split image into subimages"));
    connect(m_imageSplitAction, SIGNAL(triggered()), this, SLOT(splitToolSlot()));


    m_imageSplitWithGridAction = new QAction(tr("Split image with grid"), this);
    m_imageSplitWithGridAction->setStatusTip(tr("Allows to split image with a grid"));
    connect(m_imageSplitWithGridAction, SIGNAL(triggered()), this, SLOT(splitWithGridToolSlot()));


    m_drawCircleAction = new QAction(tr("Draw circle"), this);
    m_drawCircleAction->setStatusTip(tr("Draws a circle with specified settings"));
    connect(m_drawCircleAction, SIGNAL(triggered()), this, SLOT(circleToolSlot()));


    m_drawRectAction = new QAction(tr("Draw square"), this);
    m_drawRectAction->setStatusTip(tr("Draws a square with specified settings"));
    connect(m_drawRectAction, SIGNAL(triggered()), this, SLOT(rectToolSlot()));


    m_emphasizeWhiteAreas = new QAction(tr("Emphasize white areas"), this);
    m_emphasizeWhiteAreas->setStatusTip(tr("Emphasizes all the white areas on the image with a white line"));
    connect(m_emphasizeWhiteAreas, SIGNAL(triggered()), this, SLOT(emphasizeToolSlot()));


    // Script actions
    m_buildScriptAction = new QAction(tr("Build"), this);
    m_buildScriptAction->setStatusTip(tr("Builds the chosen cw2 script file"));
    connect(m_buildScriptAction, SIGNAL(triggered()), this, SLOT(buildScriptSlot()));


    m_buildAndRunScriptAction = new QAction(tr("Build and run"), this);
    m_buildAndRunScriptAction->setStatusTip(tr("Builds the chosen cw2 script file and executes it"));
    connect(m_buildAndRunScriptAction, SIGNAL(triggered()), this, SLOT(buildAndRunScriptSlot()));


    m_runScriptAction = new QAction(tr("Run"), this);
    m_runScriptAction->setStatusTip(tr("Runs the chosen script slot"));
    connect(m_runScriptAction, SIGNAL(triggered()), this, SLOT(runScriptSlot()));


    // Separate actions
    m_imageAction = new QAction(tr("Image"), this);
    m_imageAction->setStatusTip(tr("Information about the image"));
    connect(m_imageAction, SIGNAL(triggered()), this, SLOT(imageSlot()));

    
    m_aboutAction = new QAction(tr("Help"), this);
    m_aboutAction->setShortcut(QKeySequence::HelpContents);
    m_aboutAction->setStatusTip(tr("Information about the program"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(aboutSlot()));


    // Menus
    m_fileMenu = menuBar()->addMenu(tr("File"));
    m_fileMenu->addAction(m_newFileAction);
    m_fileMenu->addAction(m_openFileAction);
    m_fileMenu->addAction(m_saveFileAction);
    m_fileMenu->addAction(m_saveAsFileAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);


    m_toolsMenu = menuBar()->addMenu(tr("Tools"));
    m_toolsMenu->addAction(m_componentFilterAction);
    m_toolsMenu->addAction(m_imageSplitAction);
    m_toolsMenu->addAction(m_imageSplitWithGridAction);
    m_toolsMenu->addAction(m_drawCircleAction);
    m_toolsMenu->addAction(m_drawRectAction);
    m_toolsMenu->addAction(m_emphasizeWhiteAreas);


    m_scriptMenu = menuBar()->addMenu(tr("Script"));
    m_scriptMenu->addAction(m_buildScriptAction);
    m_scriptMenu->addAction(m_buildAndRunScriptAction);
    m_scriptMenu->addAction(m_runScriptAction);


    menuBar()->addAction(m_imageAction);
    menuBar()->addAction(m_aboutAction);


    // The toolbar
    m_movingToolButton = new QRadioButton();
    m_movingToolButton->setIcon(m_movingToolIcon);
    m_movingToolButton->setShortcut(Qt::Key_1);
    m_movingToolButton->setWhatsThis("Allows to move and zoom the image");
    m_movingToolButton->setChecked(true);

    m_penToolButton = new QRadioButton();
    m_penToolButton->setIcon(m_penToolIcon);
    m_penToolButton->setShortcut(Qt::Key_2);
    m_penToolButton->setWhatsThis("Allows to draw a line as if with a pen");

    m_pickToolButton = new QRadioButton();
    m_pickToolButton->setIcon(m_pickToolIcon);
    m_pickToolButton->setShortcut(Qt::Key_3);
    m_pickToolButton->setWhatsThis("Allows to pick a color from the image");


    m_toolActionsGroup = new QButtonGroup();
    m_toolActionsGroup->addButton(m_movingToolButton, (int)ToolType::MOVING);
    m_toolActionsGroup->addButton(m_penToolButton, (int)ToolType::PEN);
    m_toolActionsGroup->addButton(m_pickToolButton, (int)ToolType::PICK_COLOR);
    connect(m_toolActionsGroup, SIGNAL(idClicked(int)), this, SLOT(toolButtonClickedSlot(int)));


    m_currentColorLabel = new ColorLabel();
    connect(m_currentColorLabel, SIGNAL(clicked(int)), this, SLOT(colorLabelClickedSlot(int)));


    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addWidget(m_movingToolButton, 0, Qt::AlignLeft);
    actionLayout->addWidget(m_penToolButton, 0, Qt::AlignLeft);
    actionLayout->addWidget(m_pickToolButton, 0, Qt::AlignLeft);
    actionLayout->addWidget(createVerticalLine(2, "color: rgba(0, 5, 5, 25%)"), 0, Qt::AlignLeft);
    actionLayout->addWidget(m_currentColorLabel);

    // Hints at the bottom
    // !!! Must be initialized before the image edit scene !!!
    m_isImageSavedHint = new QLabel(this);
    m_pixelUnderCursorHint = new QLabel(this);
    m_imageSizeHint = new QLabel(this);

    
    // Image editing scene and view + tool setting slide
    m_editScene = new ImageEditScene(this, m_isImageSavedHint, m_pixelUnderCursorHint, m_imageSizeHint, m_currentColorLabel);
    m_editView = new QGraphicsView(m_editScene);
    m_editScene->setParentView(m_editView);

    connect(m_currentColorLabel, SIGNAL(colorChanged(Color)), m_editScene, SLOT(onColorChanged(Color)));

    m_toolWidthSlider = new EditSlider(nullptr, Qt::Vertical, 1, 512, 1);
    m_toolWidthSlider->setFixedWidth(45);
    m_toolWidthSlider->setHidden(true);
    m_toolWidthSlider->setValue(10);
    connect(m_toolWidthSlider, SIGNAL(valueChanged(int)), m_editScene, SLOT(onToolRadiusChanged(int)));

    QHBoxLayout* editSceneLayout = new QHBoxLayout();
    editSceneLayout->addWidget(m_editView, 15);
    editSceneLayout->addWidget(m_toolWidthSlider, 0);


    QHBoxLayout* bottomHintsLayout = new QHBoxLayout();
    bottomHintsLayout->addWidget(m_isImageSavedHint, 0, Qt::AlignLeft);
    bottomHintsLayout->addWidget(m_pixelUnderCursorHint, 0, Qt::AlignCenter);
    bottomHintsLayout->addWidget(m_imageSizeHint, 0, Qt::AlignRight);


    // Getting everything together
    QVBoxLayout* boxLayout = new QVBoxLayout();
    boxLayout->addLayout(actionLayout, 0);
    boxLayout->addLayout(editSceneLayout, 1);
    boxLayout->addLayout(bottomHintsLayout, 0);

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(boxLayout);

    setCentralWidget(centralWidget);
}

// Opens a new input dialog for two variables: width and height, and for a fill color.
// Returns true on successful enter of the values, otherwise false.
bool CW2_GraphicalEditor::newFileSizeInputDialog(QSize& size, Color& color) {
    CustomInputDialog* dialog = new CustomInputDialog(nullptr, "Creating a new file");

    dialog->addLabel(new QLabel(tr("Enter width and height of the new image."), this));

    QLineEdit* widthInput = new QLineEdit();
    widthInput->setToolTip("Width of the new image");
    widthInput->setValidator(new QIntValidator(0, 256 * 256, this));
    int widthId = dialog->addLineEdit("Width: ", widthInput);

    QLineEdit* heightInput = new QLineEdit();
    heightInput->setToolTip("Height of the new image");
    heightInput->setValidator(new QIntValidator(0, 256 * 256, this));
    int heightId = dialog->addLineEdit("Height: ", heightInput);

    ColorLabel* colorFillInput = new ColorLabel();
    colorFillInput->setColor(Color::White);
    colorFillInput->setToolTip("Fill color of the new image");
    colorFillInput->setOpenColorChoiceOnClick(true);
    int colorId = dialog->addColorLabel("Fill color: ", colorFillInput);

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        size.setWidth(dialog->getLineEditValue(widthId).toInt());
        size.setHeight(dialog->getLineEditValue(heightId).toInt());
        color = dialog->getColorLabelValue(colorId);

        return true;
    }

    return false;
}

void CW2_GraphicalEditor::errorMessage(const QString& str) {
    QMessageBox msg;
    msg.warning(this, "Error", str);
    msg.show();
}

bool CW2_GraphicalEditor::areYouSureMessage(const QString& str) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning", str, QMessageBox::Yes | QMessageBox::No);
    return reply == QMessageBox::Yes;
}

void CW2_GraphicalEditor::disableWindow() {
    /* Not used for now
    setEnabled(false);
    menuBar()->setEnabled(false);
    m_disableWindowCounter++;
    */
}

void CW2_GraphicalEditor::enableWindow() {
    /* Not used for now
    if (--m_disableWindowCounter == 0) {
        setEnabled(true);
        menuBar()->setEnabled(true);
    }
    */
}

void CW2_GraphicalEditor::newFileSlot() {
    if (m_editScene->hasUnsavedImage() &&
        !areYouSureMessage("Current image is not saved. Are you sure you want to create a new one? The current image will be lost.")) {
        return;
    }

    QSize newImageSize;
    Color fillColor;
    if (newFileSizeInputDialog(newImageSize, fillColor)) {
        if (auto result = m_editScene->newImage(newImageSize, fillColor); !result.isOk()) {
            errorMessage(result.error());
            return;
        }

        m_currentColorLabel->setOpenColorChoiceOnClick(true);
        setWindowTitle("New image : " + tr(BASIC_TITLE));
    }
}

// The callback for the "open" button in the "file" menu
void CW2_GraphicalEditor::openFileSlot() {
    if (m_editScene->hasUnsavedImage() &&
        !areYouSureMessage("Current image is not saved. Are you sure you want to open another one? The current image will be lost.")) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select the image to open"),
        QDir::currentPath(),
        "Images (*.png)" //"Images (*.png *.jpg *.jpeg *.bmp)"
    );

    if (fileName.isEmpty()) { // Nothing chosen
        return;
    }

    disableWindow();
    at_scope_exit{ enableWindow(); };

    openProgressDialog(this, "Opening a file: " + fileName);
    at_scope_exit{ closeProgressDialog(); };

    if (auto result = m_editScene->openImage(fileName); !result.isOk()) {
        errorMessage(result.error());
        return;
    }

    m_currentColorLabel->setOpenColorChoiceOnClick(true);
    setWindowTitle(QString(fileName) + " : " + tr(BASIC_TITLE));
}

void CW2_GraphicalEditor::saveFileSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to save: no image opened.");
        return;
    }

    if (!areYouSureMessage("Are you sure? This action would overwrite the original image.")) {
        return;
    }

    disableWindow();
    at_scope_exit{ enableWindow(); };

    const QString& fileName = m_editScene->getImageFileName();

    if (fileName.isEmpty()) { // A newly created file
        saveAsFileSlot();
        return;
    }

    openProgressDialog(this, "Saving the file: " + fileName);
    at_scope_exit{ closeProgressDialog(); };
    
    if (auto result = m_editScene->saveImage(fileName); !result.isOk()) {
        errorMessage(result.error());
    }
}

void CW2_GraphicalEditor::saveAsFileSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to save: no image opened.");
        return;
    }

    disableWindow();
    at_scope_exit{ enableWindow(); };

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Select the file where to save the image"),
        QDir::currentPath(),
        "Image (*.png);;Image (*.jpg);;Image (*.jpeg);;Windows Bitmap (*.bmp)"
    );

    openProgressDialog(this, "Saving the file: " + fileName);
    at_scope_exit{ closeProgressDialog(); };

    if (auto result = m_editScene->saveImage(fileName); !result.isOk()) {
        errorMessage(result.error());
    }
}

void CW2_GraphicalEditor::exitFileSlot() {
    if (m_editScene->hasUnsavedImage() &&
        !areYouSureMessage("Current image is not saved. Are you sure you want to exit? The current image will be lost.")) {
        return;
    }

    close();
}

void CW2_GraphicalEditor::filterToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Filtering by channel");

    int rId = dialog->addCheckBox("Red channel: ", new QCheckBox());
    int gId = dialog->addCheckBox("Green channel: ", new QCheckBox());
    int bId = dialog->addCheckBox("Blue channel: ", new QCheckBox());
    int aId = dialog->addCheckBox("Alpha channel: ", new QCheckBox());

    int valueId = dialog->addEditSlider("Value:", new EditSlider(nullptr, Qt::Horizontal, 0, 255, 1));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        bool red = dialog->getCheckBoxValue(rId);
        bool green = dialog->getCheckBoxValue(gId);
        bool blue = dialog->getCheckBoxValue(bId);
        bool alpha = dialog->getCheckBoxValue(aId);

        uint8_t value = (uint8_t)dialog->getEditSliderValue(valueId);

        m_editScene->channelFilter(red, green, blue, alpha, value);
    }
}

void CW2_GraphicalEditor::splitToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Splitting into subimages");

    int nId = dialog->addEditSlider(
        "Width division:",
        new EditSlider(nullptr, Qt::Horizontal, 1, m_editScene->getImage().getWidth(), 1)
    );

    int mId = dialog->addEditSlider(
        "Height division:",
        new EditSlider(nullptr, Qt::Horizontal, 1, m_editScene->getImage().getHeight(), 1)
    );

    int pathId = dialog->addFilePathEdit("Save path:", new FilePathEdit(nullptr, "", "", true));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        uint32_t N = (uint32_t)dialog->getEditSliderValue(nId);
        uint32_t M = (uint32_t)dialog->getEditSliderValue(mId);

        QString path = dialog->getFilePathEditValue(pathId);

        m_editScene->splitImageIntoSubimages(N, M, path);
    }
}

void CW2_GraphicalEditor::splitWithGridToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Splitting with a grid");

    int nId = dialog->addEditSlider(
        "Width division:",
        new EditSlider(nullptr, Qt::Horizontal, 1, m_editScene->getImage().getWidth(), 1)
    );

    int mId = dialog->addEditSlider(
        "Height division:",
        new EditSlider(nullptr, Qt::Horizontal, 1, m_editScene->getImage().getHeight(), 1)
    );

    int widthId = dialog->addEditSlider("Grid lines' width:", new EditSlider(nullptr, Qt::Horizontal, 1, 512, 1));
    int colorId = dialog->addColorLabel("Grid lines' color:", new ColorLabel(nullptr, true));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        uint32_t N = (uint32_t)dialog->getEditSliderValue(nId);
        uint32_t M = (uint32_t)dialog->getEditSliderValue(mId);
        
        float lineWidth = (float)dialog->getEditSliderValue(widthId);
        Color color = dialog->getColorLabelValue(colorId);

        m_editScene->splitImageWithGrid(N, M, lineWidth, color);
    }
}

void CW2_GraphicalEditor::circleToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Drawing a circle");

    int centerId = dialog->addPointChoiceEdit(
        "Circle's center:",
        new PointChoiceEdit(nullptr, m_editScene->getImage().getWidth(), m_editScene->getImage().getHeight())
    );

    int radiusId = dialog->addEditSlider("Circle's radius:", new EditSlider(nullptr, Qt::Horizontal, 1, 1024, 1));

    int widthId = dialog->addEditSlider("Line's width:", new EditSlider(nullptr, Qt::Horizontal, 1, 512, 1));
    int colorId = dialog->addColorLabel("Circle's color:", new ColorLabel(nullptr, true));

    int isFillId = dialog->addCheckBox("Fill the circle with filling color", new QCheckBox());
    dialog->addLabel(new QLabel("Note that fill color wouldn't do anything if the corresponding option is not checked"));

    int fillColorId = dialog->addColorLabel("Circle's fill color:", new ColorLabel(nullptr, true));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        QPoint center = dialog->getPointChoiceEditValue(centerId);
        float radius = (float)dialog->getEditSliderValue(radiusId);

        float lineWidth = (float)dialog->getEditSliderValue(widthId);
        Color color = dialog->getColorLabelValue(colorId);

        bool isFilled = dialog->getCheckBoxValue(isFillId);
        Color fillColor = dialog->getColorLabelValue(fillColorId);

        m_editScene->drawComplexCircle(center, radius, lineWidth, color, fillColor, isFilled);
    }
}

void CW2_GraphicalEditor::rectToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Drawing a rectangle");

    dialog->addLabel(new QLabel("Draws a rectangle and its diagonals"));

    int fromId = dialog->addPointChoiceEdit(
        "Left-top point:",
        new PointChoiceEdit(nullptr, m_editScene->getImage().getWidth(), m_editScene->getImage().getHeight())
    );

    int sideId = dialog->addEditSlider("Rectangle's side:", new EditSlider(nullptr, Qt::Horizontal, 1, 1024, 1));

    int widthId = dialog->addEditSlider("Line's width:", new EditSlider(nullptr, Qt::Horizontal, 1, 512, 1));
    int colorId = dialog->addColorLabel("Rectangle's color:", new ColorLabel(nullptr, true));

    int isFillId = dialog->addCheckBox("Fill the rectangle with filling color", new QCheckBox());
    dialog->addLabel(new QLabel("Note that fill color wouldn't do anything if the corresponding option is not checked"));

    int fillColorId = dialog->addColorLabel("Rectangle's fill color:", new ColorLabel(nullptr, true));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        QPoint from = dialog->getPointChoiceEditValue(fromId);
        uint32_t side = (uint32_t)dialog->getEditSliderValue(sideId);

        float lineWidth = (float)dialog->getEditSliderValue(widthId);
        Color color = dialog->getColorLabelValue(colorId);

        bool isFilled = dialog->getCheckBoxValue(isFillId);
        Color fillColor = dialog->getColorLabelValue(fillColorId);

        m_editScene->drawSquareWithDiagonals(from, side, lineWidth, color, fillColor, isFilled);
    }
}

void CW2_GraphicalEditor::emphasizeToolSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("Nothing to edit: no image opened.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Drawing a rectangle");

    int colorId = dialog->addColorLabel("Line's color:", new ColorLabel(nullptr, true));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        Color color = dialog->getColorLabelValue(colorId);

        m_editScene->emphasizeWhiteAreas(color);
    }
}

void CW2_GraphicalEditor::buildScriptSlot() {
    CustomInputDialog* dialog = new CustomInputDialog(this, "Building a script");

    int fileId = dialog->addFilePathEdit("CW2 script file:", new FilePathEdit(nullptr, "", "CW2 script (*.cw2)", false));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        QString scriptPath = dialog->getFilePathEditValue(fileId);

        Compiler compiler;
        if (auto result = compiler.compile(scriptPath); !result.isOk()) {
            errorMessage(result.error());
        } else {
            QMessageBox msg;
            msg.information(this, "CW2 compiler", "Build successful: the compiled script is stored to " + scriptPath + "c");
            msg.show();
        }
    }
}

void CW2_GraphicalEditor::buildAndRunScriptSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("No image opened. Nowhere to execute the script on.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Building and running a script");

    int fileId = dialog->addFilePathEdit("CW2 script file:", new FilePathEdit(nullptr, "", "CW2 script (*.cw2)", false));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        QString scriptPath = dialog->getFilePathEditValue(fileId);

        Compiler compiler;
        if (auto result = compiler.compile(scriptPath); !result.isOk()) {
            errorMessage(result.error());
        } else {
            QMessageBox msg;
            msg.information(this, "CW2 compiler", "Build successful: the compiled script is stored to " + scriptPath + "c");

            // Executing
            if (m_cw2VM == nullptr) {
                m_cw2VM = new CW2VM(m_editScene);
            } else {
                m_cw2VM->forceHalt();
            }

            if (auto result = m_cw2VM->loadFromFile(scriptPath + "c"); !result.isOk()) {
                errorMessage(result.error());
            } else {
                m_cw2VM->execute();
            }
        }
    }
}

void CW2_GraphicalEditor::runScriptSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("No image opened. Nowhere to execute the script on.");
        return;
    }

    CustomInputDialog* dialog = new CustomInputDialog(this, "Running a script");

    int fileId = dialog->addFilePathEdit("CW2 compiled script file:", new FilePathEdit(nullptr, "", "CW2 compiled script (*.cw2c)", false));

    dialog->finishSetupUI();

    if (dialog->exec() == QDialog::Accepted) {
        QString scriptPath = dialog->getFilePathEditValue(fileId);

        if (m_cw2VM == nullptr) {
            m_cw2VM = new CW2VM(m_editScene);
        } else {
            m_cw2VM->forceHalt();
        }

        if (auto result = m_cw2VM->loadFromFile(scriptPath); !result.isOk()) {
            errorMessage(result.error());
        } else {
            m_cw2VM->execute();
        }
    }
}

void CW2_GraphicalEditor::imageSlot() {
    if (!m_editScene->isImageOpened()) {
        errorMessage("No image opened. Open an image or create a new one before using this menu.");
        return;
    }

    ImageWindow imageInfo = ImageWindow(this, m_editScene->getImage().getFullImageInformation());
    imageInfo.show();

    // Waiting till the image window is closed
    QEventLoop eventLoop;
    connect(&imageInfo, SIGNAL(destroyed()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void CW2_GraphicalEditor::aboutSlot() {
    HelpWindow about = HelpWindow(this);
    if (auto result = about.openHtml(ABOUT_HTML_FILE_PATH); !result.isOk()) {
        errorMessage(result.error());
    }

    about.show();
    
    // Waiting till the about window is closed
    QEventLoop eventLoop;
    connect(&about, SIGNAL(destroyed()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void CW2_GraphicalEditor::toolButtonClickedSlot(int id) {
    if (!m_editScene->isImageOpened()) {
        m_movingToolButton->setChecked(true);
        return;
    }

    m_editScene->setToolType((ToolType)id);
    if (hasWidthAttribute((ToolType)id)) {
        m_toolWidthSlider->setHidden(false);
    } else {
        m_toolWidthSlider->setHidden(true);
    }
}

void CW2_GraphicalEditor::closeEvent(QCloseEvent* event) {
    exitFileSlot();
    event->ignore();
}
