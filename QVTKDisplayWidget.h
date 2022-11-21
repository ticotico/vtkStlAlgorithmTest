#pragma once
#include <memory>
#include <type_traits>
#include <array>
#include <tuple>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include "QVTKWidget.h"

#include <QBoxLayout>

template<typename ...Args>
class QVTKDisplayWidget
{
	using TupleItemType = std::tuple<vtkSmartPointer<Args>...>;

public:
	static constexpr std::size_t ArgsCount = sizeof...(Args);

	QVTKDisplayWidget(QWidget* parent = nullptr)
		: m_upWidget(new QWidget(parent))
		, m_upViewer(new QVTKWidget)
		, m_pRenderWindow(vtkSmartPointer<vtkRenderWindow>::New())
		, m_pRenderer(vtkSmartPointer<vtkRenderer>::New())
		, m_Items(std::make_tuple(vtkSmartPointer<Args>::New()...))
	{
		m_upViewer->SetRenderWindow(m_pRenderWindow);
		m_pRenderWindow->AddRenderer(m_pRenderer);

		auto pLayout = new QHBoxLayout(m_upWidget.get());
		pLayout->setMargin(0);
		pLayout->setSpacing(0);
		pLayout->addWidget(m_upViewer.get());
		m_upWidget->setLayout(pLayout);

		InitializeMapperAndAcotr(std::make_index_sequence<ArgsCount/2>());
	}

	QWidget* Widget()
	{
		return m_upWidget.get();
	}

	vtkSmartPointer<vtkRenderer> Renderer()
	{
		return m_pRenderer;
	}

	~QVTKDisplayWidget()
	{
		m_pRenderer->RemoveAllViewProps();
	}

	void Render()
	{
		m_pRenderer->GetRenderWindow()->GetInteractor()->Render();
	}

	void SetInteractorStyle(vtkSmartPointer<vtkInteractorStyle> pStyle)
	{
		m_pRenderer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pStyle);
	}

	template<std::size_t Idx>
	auto Mapper()
	{
		return std::get<2*Idx>(m_Items);
	}

	template<std::size_t Idx>
	auto Actor()
	{
		return std::get<2*Idx+1>(m_Items);
	}

private:
	template<std::size_t ...Idxs>
	void InitializeMapperAndAcotr(std::index_sequence<Idxs...>)
	{
		(..., (m_pRenderer->AddActor(Actor<Idxs>())));
		(..., (Actor<Idxs>()->SetMapper(Mapper<Idxs>())));
	}

private:
	std::unique_ptr<QWidget> m_upWidget;
	std::unique_ptr<QVTKWidget> m_upViewer;
	vtkSmartPointer<vtkRenderWindow> m_pRenderWindow;
	vtkSmartPointer<vtkRenderer> m_pRenderer;

	TupleItemType m_Items;
};
