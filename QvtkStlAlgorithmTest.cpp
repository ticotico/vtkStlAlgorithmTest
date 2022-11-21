#include "QvtkStlAlgorithmTest.h"
#include "vtkSTLReader.h"
#include "InteractorStyleMouseListener.h"

QvtkStlAlgorithmTest::QvtkStlAlgorithmTest(QWidget *parent)
    : QMainWindow(parent)
    , m_displayWidget(new DisplayWidgetType)
    , m_mouseListener(vtkSmartPointer<InteractorStyleMouseListener>::New())
{
    ui.setupUi(this);
    ui.m_layout->addWidget(m_displayWidget->Widget());
	m_mouseListener->SetPickRender(m_displayWidget->Renderer());
    m_displayWidget->SetInteractorStyle(m_mouseListener);
    m_mouseListener->RegisterCallbackFunctionMouseLeftClicked(std::bind(&QvtkStlAlgorithmTest::OnMouseLeftClick, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4
    ));


    auto stlReader = vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName("sample1.stl");
    stlReader->Update();
    m_displayWidget->Mapper<0>()->SetInputData(stlReader->GetOutput());
}

void QvtkStlAlgorithmTest::OnMouseLeftClick(bool b, double x, double y, double z)
{
	std::cout << x << ", " << y << ", " << z << std::endl;
}
