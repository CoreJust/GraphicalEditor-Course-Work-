#pragma once
#include <QtWidgets/QMainWindow>
#include <QtGui/QAction>
#include <QtWidgets/QToolBar.h>
#include <QtWidgets/QMenu.h>
#include <QtWidgets/QMenuBar.h>
#include <QtWidgets/QGraphicsView.h>
#include <QButtonGroup>
#include <QRadioButton>
#include "EditSlider.h"
#include "ImageEditScene.h"
#include "ColorLabel.h"
#include "../Script/CW2VM.h"

class CW2_GraphicalEditor final : public QMainWindow {
    Q_OBJECT

private:
    // The area where the image is displayed and being edited
    ImageEditScene* m_editScene;
    QGraphicsView* m_editView;

    // The CW2 script virtual machine
    CW2VM* m_cw2VM = nullptr;

    // The upper menus
    QMenu* m_fileMenu;
    QMenu* m_toolsMenu;
    QMenu* m_scriptMenu;

    // Actions in the file menu
    QAction* m_newFileAction;
    QAction* m_openFileAction;
    QAction* m_saveFileAction;
    QAction* m_saveAsFileAction;
    QAction* m_exitAction;

    // Actions in the tool menu
    QAction* m_componentFilterAction;
    QAction* m_imageSplitAction;
    QAction* m_imageSplitWithGridAction;
    QAction* m_drawCircleAction;
    QAction* m_drawRectAction;
    QAction* m_emphasizeWhiteAreas;

    // Actions in the script menu
    QAction* m_buildScriptAction;
    QAction* m_buildAndRunScriptAction;
    QAction* m_runScriptAction;

    // Independent buttons in the upper menubar
    QAction* m_imageAction;
    QAction* m_aboutAction;

    // The group of buttons representing the tool choice (some of them)
    QButtonGroup* m_toolActionsGroup;

    QRadioButton* m_movingToolButton;
    QRadioButton* m_penToolButton;
    QRadioButton* m_pickToolButton;

    ColorLabel* m_currentColorLabel;

    // The slider at the right controling tool's width
    EditSlider* m_toolWidthSlider;

    // The hints at the bottom of the window
    QLabel* m_isImageSavedHint;
    QLabel* m_pixelUnderCursorHint;
    QLabel* m_imageSizeHint;

    // Icons
    QIcon m_movingToolIcon;
    QIcon m_penToolIcon;
    QIcon m_pickToolIcon;

    size_t m_disableWindowCounter = 0;


public:
    CW2_GraphicalEditor(QWidget *parent = nullptr);
    ~CW2_GraphicalEditor();

private:
    void loadIcons();
    void setupUI();

    bool newFileSizeInputDialog(QSize& size, Color& color);

    void errorMessage(const QString& str);
    bool areYouSureMessage(const QString& str);

    void disableWindow();
    void enableWindow();

private slots:
    // File slots
    void newFileSlot();
    void openFileSlot();
    void saveFileSlot();
    void saveAsFileSlot();
    void exitFileSlot();

    // Tool slots
    void filterToolSlot();
    void splitToolSlot();
    void splitWithGridToolSlot();
    void circleToolSlot();
    void rectToolSlot();
    void emphasizeToolSlot();

    // Script slots
    void buildScriptSlot();
    void buildAndRunScriptSlot();
    void runScriptSlot();

    // Independent slots
    void imageSlot();
    void aboutSlot();

    void toolButtonClickedSlot(int id);

private: // events
    void closeEvent(QCloseEvent* event) override;
};
