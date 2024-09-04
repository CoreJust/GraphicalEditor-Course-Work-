#include "GUI/CW2_GraphicalEditor.h"
#include <QtWidgets/QApplication>
#include "Image/Color.h"

/*
* Changelog (and some notes):
*   06.05.2023: Created the project, started studying Qt, added main window, added toolbar with unusable buttons
*               (except for "open"), added image opening, added zooming using the mouse wheel.
*   07.05.2023: Added some comments to make the code more clear, fixed the size at opening (it was the minimal possible),
*               added drag & dropping.
*   08.05.2023: Fixed the bug when you move the image while the image is out of the visible scene,
*               added error's dispaying via QMessageBox.
*   09.05.2023: Added warnings regarding unsaved changes, added exiting, new image creation, saving images.
*   11.05.2023: Changed QVBoxLayout to QFormLayout in input dialogs, images are now scaled to 
*               fit in view on opening/creating, added some preparations for editing tools, added help - main page.
*   12.05.2023: Implemented own class for handling the images, now works with png, read-only (now using libpng)
*   15.05.2023: Fixed the bug with color being changed while passing to Qt, partially rewritten some code (made better error handling),
*               added png-file saving with own image implementation, improved help
*   16.05.2023: Expanded the help, added russian version, added image information window, added information bar at the bottom
*   18.05.2023: Fixed a memory leak on saving image, added progress bar on opening/saving images, primitive pen
*   20.05.2023: Added some internal rendering stuff (rasterizers, I'm pretty proud of them), now a complete pen tool,
*               implemented pick-color tool, added background in the edit scene
*   21.05.2023: Color choice menu (own, without QColorDialog), some minor bug fixes, added tool's width slider
*   22.05.2023: Implemented all the required features, finished the help, started implementing CW2 script language
*   23.05.2023: Obviously, implementing CW2 script language would take a whole day, so I stopped at a "raw" version
*               and erased all the reference to it from other modules, since I have also to write a report and make PR today
*   02.06.2023: Finished the first version of CW2 script language, now can draw simple images/animations
*/

// TODOs: tIME and other info, square areas, eraser, image cropping, image rotation, finish CW2 script language

int main(int argc, char *argv[]) {
    Color::initColorModule(); // Getting ready to use own graphics

    // The standard Qt project's main function content
    QApplication a(argc, argv);
    CW2_GraphicalEditor w;
    w.show();

    return a.exec();
}
