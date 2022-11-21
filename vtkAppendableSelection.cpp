#include "vtkAppendableSelection.h"

#include <set>
#include <queue>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkGeometryFilter.h>
#include <vtkPointLocator.h>

vtkStandardNewMacro(vtkAppendableSelection);

static bool GetCellIdsInRegion(vtkPolyData* polydata, std::array<double, 3> pos3d, double radius, vtkIdList* outIds)
{
	// input check
	if(!polydata|| radius <= 0.0 || !outIds)
		return false;
	std::set<vtkIdType> visitedCells;

	auto pointLocator = vtkSmartPointer<vtkPointLocator>::New();
	pointLocator->SetDataSet(polydata);
	pointLocator->AutomaticOn();
	pointLocator->SetNumberOfPointsPerBucket(2);
	pointLocator->BuildLocator();

	auto pointsInRadius = vtkSmartPointer<vtkIdList>::New();
	pointLocator->FindPointsWithinRadius(radius, pos3d.data(), pointsInRadius);

	const vtkIdType pointCnt = pointsInRadius->GetNumberOfIds();
	vtkSmartPointer<vtkIdList> tmpCells = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> tmpCellPoints = vtkSmartPointer<vtkIdList>::New();
	for(vtkIdType i = 0; i < pointCnt; ++i)
	{
		tmpCells->Initialize();
		polydata->GetPointCells(pointsInRadius->GetId(i), tmpCells);
		const vtkIdType cellCount = tmpCells->GetNumberOfIds();
		for(vtkIdType j = 0; j < cellCount; ++j)
		{
			const vtkIdType cellId = tmpCells->GetId(j);
			if(visitedCells.count(cellId) == 0)
			{
				visitedCells.insert(cellId);
				tmpCellPoints->Initialize();
				polydata->GetCellPoints(cellId, tmpCellPoints);
				bool allVertexWithinRadius(true);
				for(int k = 0; k < tmpCellPoints->GetNumberOfIds(); ++k)
				{
					if(-1 == pointsInRadius->IsId(tmpCellPoints->GetId(k)))
					{
						allVertexWithinRadius = false;
						break;
					}
				}
				if(allVertexWithinRadius)
					outIds->InsertUniqueId(cellId);
			}
		}
	}
	return true;
}

vtkSmartPointer<vtkPolyData> GetCellsPolyData(vtkPolyData* pd, vtkIdTypeArray* ids)
{
	auto ret = vtkSmartPointer<vtkPolyData>::New();
	if(pd && ids)
	{
		auto selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
		selectionNode->SetFieldType(vtkSelectionNode::CELL);	
		selectionNode->SetContentType(vtkSelectionNode::INDICES);
		selectionNode->SetSelectionList(ids);
		selectionNode->GetProperties()->Set(vtkSelectionNode::CONTAINING_CELLS(), 1);
		selectionNode->GetProperties()->Set(vtkSelectionNode::INVERSE(), 0);

		auto selection = vtkSmartPointer<vtkSelection>::New();
		selection->AddNode(selectionNode);

		auto extractSelection = vtkSmartPointer<vtkExtractSelection>::New();
		extractSelection->SetInputData(0, pd);
		extractSelection->SetInputData(1, selection);
		extractSelection->Update();

		auto geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
		geometryFilter->SetInputData(extractSelection->GetOutput());
		geometryFilter->Update();

		ret->ShallowCopy(geometryFilter->GetOutput());
	}

	return ret;
}

vtkAppendableSelection::vtkAppendableSelection()
	: vtkPolyDataAlgorithm()
	, m_pos3d({0.0, 0.0, 0.0})
	, m_dRadius(0.0)
	, m_bAppend(false)
	, m_bSetSelection(false)
	, m_bClearSelection(false)
	, m_selectedRegion(vtkSmartPointer<vtkIdList>::New())
	, m_applyRegion(vtkSmartPointer<vtkIdList>::New())
{
    SetNumberOfInputPorts(1);
    SetNumberOfOutputPorts(2);
}

void vtkAppendableSelection::SelectionSetting(std::array<double, 3> pos3d, double radius, bool append)
{
	std::copy(pos3d.cbegin(), pos3d.cend(), m_pos3d.begin());
	m_dRadius = radius;
	m_bAppend = append;

	m_bSetSelection = true;
	this->Modified();
}

void vtkAppendableSelection::NoAppendSelection(std::array<double, 3> pos3d, double radius)
{
	SelectionSetting(pos3d, radius, false);
}

void vtkAppendableSelection::AppendSelection(std::array<double, 3> pos3d, double radius)
{
	SelectionSetting(pos3d, radius, true);
}

void vtkAppendableSelection::ClearSelection()
{
	m_bClearSelection = true;
	m_selectedCeneters.clear();
	this->Modified();
}

vtkSmartPointer<vtkIdList> vtkAppendableSelection::GetAppliedRegionIds()
{
	return m_applyRegion;
}

std::vector<std::array<double, 3>> vtkAppendableSelection::SelectedCenters() const
{
	return m_selectedCeneters;
}

int vtkAppendableSelection::ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
	if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
	{
		vtkInformation *inInfoA = inputVector[0]->GetInformationObject(0);
		vtkPolyData *pdA = vtkPolyData::SafeDownCast(inInfoA->Get(vtkDataObject::DATA_OBJECT()));
		vtkInformation *outInfoA = outputVector->GetInformationObject(0);
		vtkPolyData *resultA = vtkPolyData::SafeDownCast(outInfoA->Get(vtkDataObject::DATA_OBJECT()));
		vtkInformation *outInfoB = outputVector->GetInformationObject(1);
		vtkPolyData *resultB = vtkPolyData::SafeDownCast(outInfoB->Get(vtkDataObject::DATA_OBJECT()));

		if(m_bClearSelection)
		{
			m_bClearSelection = false;
			m_selectedRegion->Initialize();
			m_applyRegion->Initialize();
			resultA->Initialize();
			resultB->Initialize();
		}
		else if(m_bSetSelection)
		{
			m_bSetSelection = false;
			if(m_dRadius > 0.0)
			{
				m_selectedRegion->Initialize();
				GetCellIdsInRegion(pdA, m_pos3d, m_dRadius, m_selectedRegion);
				if(m_bAppend)
				{
					for(vtkIdType i = 0; i < m_selectedRegion->GetNumberOfIds(); ++i)
					{
						m_applyRegion->InsertUniqueId(m_selectedRegion->GetId(i));
					}
					if(m_selectedRegion->GetNumberOfIds() > 0)
						m_selectedCeneters.push_back(m_pos3d);
				}

				{
					auto ids = vtkSmartPointer<vtkIdTypeArray>::New();
					for(vtkIdType i = 0; i < m_selectedRegion->GetNumberOfIds(); ++i)
						ids->InsertNextValue(m_selectedRegion->GetId(i));
					auto selectedPolyData = GetCellsPolyData(pdA, ids);
					resultA->ShallowCopy(selectedPolyData);
				}
				{
					auto ids = vtkSmartPointer<vtkIdTypeArray>::New();
					for(vtkIdType i = 0; i < m_applyRegion->GetNumberOfIds(); ++i)
						ids->InsertNextValue(m_applyRegion->GetId(i));
					auto applyPolyData = GetCellsPolyData(pdA, ids);
					resultB->ShallowCopy(applyPolyData);
				}
			}
		}
		else //other case, maybe inputpoly data modified
		{
			m_selectedRegion->Initialize();
			m_applyRegion->Initialize();
			resultA->Initialize();
			resultB->Initialize();
		}
	}
	return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}
