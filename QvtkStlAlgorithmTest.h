#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QvtkStlAlgorithmTest.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "QVTKDisplayWidget.h"

class vtkPolyDataMapper;
class vtkActor;
class InteractorStyleMouseListener;
class vtkSTLReader;
class vtkConnectivityFilter;
class vtkPointLocator;

class QvtkStlAlgorithmTest : public QMainWindow
{
    Q_OBJECT

public:
    QvtkStlAlgorithmTest(QWidget *parent = Q_NULLPTR);

private:
    void OnMouseLeftClick(bool b, double x, double y, double z);
private:
    Ui::QvtkStlAlgorithmTestClass ui;
    using DisplayWidgetType = QVTKDisplayWidget<vtkPolyDataMapper, vtkActor>;
    std::unique_ptr<DisplayWidgetType> m_displayWidget;
    vtkSmartPointer<InteractorStyleMouseListener> m_mouseListener;
    vtkSmartPointer<vtkSTLReader> m_stlReader;
    vtkSmartPointer<vtkConnectivityFilter> m_connectivity;
    vtkSmartPointer<vtkPointLocator> m_locator;
};
