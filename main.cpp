#include "QvtkStlAlgorithmTest.h"
#include <QtWidgets/QApplication>
#include "vtkAutoInit.h" 

int main(int argc, char *argv[])
{
    VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
    VTK_MODULE_INIT(vtkInteractionStyle);
    QApplication a(argc, argv);
    QvtkStlAlgorithmTest w;
    w.show();
    return a.exec();
}
