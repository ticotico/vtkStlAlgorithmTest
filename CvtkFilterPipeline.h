#pragma once
#include <tuple>
#include <vtkSmartPointer.h>
#include <functional>

template<class ...Args>
class CvtkFilterPipeline
{
public:
	using tuple_of_filter_type = std::tuple<vtkSmartPointer<Args>...>;

	CvtkFilterPipeline()
		: m_tuple(std::make_tuple(vtkSmartPointer<Args>::New()...))
	{
		if constexpr (sizeof...(Args) > 1)
			ConnectAllFilterImpl(std::make_index_sequence<sizeof...(Args) - 1>());
	}

	template<std::size_t N>
	auto GetFilter()
	{
		return std::get<N>(m_tuple);
	}

	template<typename T>
	auto GetFilter()
	{
		return std::get<vtkSmartPointer<T>>(m_tuple);
	}

	void UpdateAllFilter()
	{
		UpdateAllFilterImpl(std::make_index_sequence<sizeof...(Args)>());
	}

private:
	template<std::size_t ... Idxs>
	void ConnectAllFilterImpl(std::index_sequence<Idxs...>)
	{
		(std::get<Idxs+1>(m_tuple)->SetInputData(std::get<Idxs>(m_tuple)->GetOutput()), ...);
	}

	template<std::size_t ... Idxs>
	void UpdateAllFilterImpl(std::index_sequence<Idxs...>)
	{
		(std::get<Idxs>(m_tuple)->Update(), ...);
	}

private:
	tuple_of_filter_type m_tuple;
};