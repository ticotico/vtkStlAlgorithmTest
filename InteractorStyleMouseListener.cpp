#include "InteractorStyleMouseListener.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPropPicker.h>
#include <vtkWorldPointPicker.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(InteractorStyleMouseListener);

InteractorStyleMouseListener::InteractorStyleMouseListener()
	: vtkInteractorStyleTrackballCamera()
	, m_bMouseLeftDown(false)
	, m_bMouseRightDown(false)
	, m_pPickRender(vtkSmartPointer<vtkRenderer>())
	, m_cbMousePosUpdate(nullptr)
	, m_cbMouseLeftClicked(nullptr)
{
}

void InteractorStyleMouseListener::OnLeftButtonDown()
{
	m_bMouseLeftDown = true;
	auto ret = GetSpacePositionByMouse();
	if (ret.has_value() && m_cbMouseLeftClicked)
	{
		std::apply(m_cbMouseLeftClicked, ret.value());
	}
	//vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void InteractorStyleMouseListener::OnLeftButtonUp()
{
	m_bMouseLeftDown = false;
	//vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

void InteractorStyleMouseListener::OnRightButtonDown()
{
	m_bMouseRightDown = true;
	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
	//vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

void InteractorStyleMouseListener::OnRightButtonUp()
{
	m_bMouseRightDown = false;
	vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
	//vtkInteractorStyleTrackballCamera::OnRightButtonUp();
}

void InteractorStyleMouseListener::OnMouseMove()
{
	auto ret = GetSpacePositionByMouse();
	if (ret.has_value() && m_cbMousePosUpdate)
	{
		std::apply(m_cbMousePosUpdate, ret.value());
	}
	vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void InteractorStyleMouseListener::SetPickRender(vtkSmartPointer<vtkRenderer> render)
{
	m_pPickRender = render;
}

void InteractorStyleMouseListener::RegisterCallbackFunctionMousePositionUpdate(CallbackFunctionType cb)
{
	m_cbMousePosUpdate = cb;
}

void InteractorStyleMouseListener::RegisterCallbackFunctionMouseLeftClicked(CallbackFunctionType cb)
{
	m_cbMouseLeftClicked = cb;
}

bool InteractorStyleMouseListener::IsMouseLeftDown() const
{
	return m_bMouseLeftDown;
}

bool InteractorStyleMouseListener::IsMouseRightDown() const
{
	return m_bMouseRightDown;
}

std::optional<std::tuple<bool,double,double,double>> InteractorStyleMouseListener::GetSpacePositionByMouse()
{
	if (!m_pPickRender)
		return std::nullopt;
	int mouseEventPosition[2] = { 0 };
	double pickerPosition[3] = { 0.0 };
	bool anyObjectsPicked(false);

	this->GetInteractor()->GetEventPosition(mouseEventPosition);
	auto propPicker = vtkSmartPointer<vtkPropPicker>::New();
	propPicker->Pick(mouseEventPosition[0], mouseEventPosition[1], 0.0, m_pPickRender);
	if (propPicker->GetActor())
	{
		propPicker->GetPickPosition(pickerPosition);
		anyObjectsPicked = true;
	}
	else
	{
		auto picker = vtkSmartPointer<vtkWorldPointPicker>::New();
		picker->Pick(mouseEventPosition[0], mouseEventPosition[1], 0.0, m_pPickRender);
		picker->GetPickPosition(pickerPosition);
	}
	return std::make_tuple(anyObjectsPicked, pickerPosition[0], pickerPosition[1], pickerPosition[2]);
}
