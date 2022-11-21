#pragma once
#include <functional>
#include <tuple>
#include <optional>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>

class vtkRenderer;
class InteractorStyleMouseListener : public vtkInteractorStyleTrackballCamera
{
protected:
	InteractorStyleMouseListener();
	~InteractorStyleMouseListener() override = default;

public:
	static InteractorStyleMouseListener* New();
	vtkTypeMacro(InteractorStyleMouseListener, vtkInteractorStyleTrackballCamera);

	virtual void OnLeftButtonDown() override;
	virtual void OnLeftButtonUp() override;
	virtual void OnRightButtonDown() override;
	virtual void OnRightButtonUp() override;
	virtual void OnMouseMove() override;
	void SetPickRender(vtkSmartPointer<vtkRenderer> render);
	using CallbackFunctionType = std::function<void(bool, double, double, double)>;
	void RegisterCallbackFunctionMousePositionUpdate(CallbackFunctionType cb);
	void RegisterCallbackFunctionMouseLeftClicked(CallbackFunctionType cb);
	bool IsMouseLeftDown() const;
	bool IsMouseRightDown() const;

private:
	std::optional<std::tuple<bool,double,double,double>> GetSpacePositionByMouse();

private:
	bool m_bMouseLeftDown;
	bool m_bMouseRightDown;
	vtkSmartPointer<vtkRenderer> m_pPickRender;
	CallbackFunctionType m_cbMousePosUpdate;//回傳滑鼠對應data的空間座標
	CallbackFunctionType m_cbMouseLeftClicked;//回傳滑鼠對應data的空間座標
};