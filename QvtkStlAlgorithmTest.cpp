#include "QvtkStlAlgorithmTest.h"
#include "vtkSTLReader.h"
#include "InteractorStyleMouseListener.h"
#include <vtkConnectivityFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>

QvtkStlAlgorithmTest::QvtkStlAlgorithmTest(QWidget* parent)
    : QMainWindow(parent)
    , m_displayWidget(new DisplayWidgetType)
    , m_mouseListener(vtkSmartPointer<InteractorStyleMouseListener>::New())
    , m_stlReader(vtkSmartPointer<vtkSTLReader>::New())
    , m_connectivity(vtkSmartPointer<vtkConnectivityFilter>::New())
    , m_locator(vtkSmartPointer<vtkPointLocator>::New())
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


    m_stlReader->SetFileName("sample1.stl");
    m_stlReader->Update();
    m_displayWidget->Mapper<0>()->SetInputData(m_stlReader->GetOutput());

    m_connectivity->SetInputData(m_stlReader->GetOutput());
    m_connectivity->SetExtractionModeToAllRegions();
    m_connectivity->ColorRegionsOn();
    m_connectivity->Update();


	m_locator->SetDataSet(m_connectivity->GetOutput());
	m_locator->AutomaticOn();
	m_locator->SetNumberOfPointsPerBucket(2);
	m_locator->BuildLocator();
}

void QvtkStlAlgorithmTest::OnMouseLeftClick(bool b, double x, double y, double z)
{
    if (b)
    {

		double xyz[3] = { x,y,z };
		auto id = m_locator->FindClosestPoint(xyz);
        std::cout << "(" << x << ", " << y << ", " << z << ")";
		std::cout << "id=" << m_connectivity->GetOutput()->GetPointData()->GetArray("RegionId")->GetTuple1(id) << std::endl;

    }
}
