#include "vtkHelperFunctions.h"
#include <iterator>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkPolyDataNormals.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkTriangle.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkLineSource.h>
#include <vtkTubeFilter.h>

#include <array>
#include <map>
#include <set>
#include <algorithm>

void printPolydataInformation(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata)
{
	auto numPoint = polydata->GetNumberOfPoints();
	auto numCell = polydata->GetNumberOfCells();
	os << "There are " << numPoint << " points" << std::endl;
	os << "There are " << numCell << " cells" << std::endl;
}

void printPolydataDetail(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata)
{
	auto numPoint = polydata->GetNumberOfPoints();
	os << "There are " << numPoint << " points" << std::endl;
	for(int i =0; i < numPoint; ++i)
	{
		os << polydata->GetPoint(i)[0] << ", ";
		os << polydata->GetPoint(i)[1] << ", ";
		os << polydata->GetPoint(i)[2] << ", " << std::endl;
	}
	auto numCell = polydata->GetNumberOfCells();
	os << "There are " << numCell << " cells" << std::endl;
	for(int i = 0; i < numCell; ++i)
	{
		auto ids = polydata->GetCell(i)->GetPointIds();
		for (int j = 0; j < polydata->GetCell(i)->GetNumberOfPoints(); ++j)
		{
			os << ids->GetId(j) << ", ";
		}
		os << std::endl;
	}
}

void cleanPolydata(vtkSmartPointer<vtkPolyData> polydata)
{
	if(polydata)
	{
		auto triangle = vtkSmartPointer<vtkTriangleFilter>::New();
		auto cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
		if(triangle && cleaner)
		{
			cleaner->SetInputData(polydata);
			cleaner->Update();
			cleaner->PointMergingOn();
			cleaner->ConvertPolysToLinesOn();
			cleaner->ConvertStripsToPolysOn();
			cleaner->ToleranceIsAbsoluteOff();
			triangle->SetInputData(cleaner->GetOutput());
			triangle->Update();
			polydata->ShallowCopy(triangle->GetOutput());
		}
	}
}

bool isManifold(vtkSmartPointer<vtkPolyData> polydata)
{
	if(polydata)
	{
		auto feature = vtkSmartPointer<vtkFeatureEdges>::New();
		feature->SetInputData(polydata);
		feature->BoundaryEdgesOff();
		feature->FeatureEdgesOff();
		feature->ManifoldEdgesOff();
		feature->NonManifoldEdgesOn();
		feature->Update();
		const auto numPoint = feature->GetOutput()->GetNumberOfPoints();
		return numPoint == 0;
	}
	else
	{
		return false;
	}
}

vtkSmartPointer<vtkPolyData> rebuildPolyData(vtkSmartPointer<vtkPolyData> polydata)
{
	vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();

	auto tri = vtkSmartPointer<vtkTriangleFilter>::New();
	tri->SetInputData(polydata);
	tri->Update();

	auto points = tri->GetOutput()->GetPoints();
	auto num_cell = tri->GetOutput()->GetNumberOfCells();
	auto cells = vtkSmartPointer<vtkCellArray>::New();

	if(points && cells && num_cell > 0)
	{
		for(vtkIdType i = 0; i < num_cell; ++i)
		{
			auto cell = tri->GetOutput()->GetCell(i);
			if(cell->GetCellType() == VTK_TRIANGLE)
				cells->InsertNextCell(cell);
		}

		auto usg = vtkSmartPointer<vtkUnstructuredGrid>::New();
		usg->SetPoints(points);
		usg->SetCells(VTK_TRIANGLE, cells);

		vtkSmartPointer<vtkDataSetSurfaceFilter> surfacefilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
		if(points->GetNumberOfPoints() != 0 && cells->GetNumberOfCells() != 0)
		{
			surfacefilter->SetInputData(usg);
			surfacefilter->Update();
		}

		ret->ShallowCopy(surfacefilter->GetOutput());
	}

	return ret;
}

void computeNormals(vtkSmartPointer<vtkPolyData> polydata)
{
	vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
	normalGenerator->SetInputData(polydata);
	normalGenerator->ComputePointNormalsOn();
	normalGenerator->ComputeCellNormalsOn();
	normalGenerator->ConsistencyOn();
	normalGenerator->NonManifoldTraversalOn();
	normalGenerator->AutoOrientNormalsOn();
	normalGenerator->Update();
	polydata->ShallowCopy(normalGenerator->GetOutput());
}

std::array<double, 3> computeSelectedCellsNormal(vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkIdList> slectRegion)
{
    std::array<double, 3> totalNormal{0, 0, 0};
    for (vtkIdType i = 0; i < slectRegion->GetNumberOfIds(); i++)
    {
        vtkIdType cellId = slectRegion->GetId(i);
        vtkIdType nPoints;
        vtkIdType* pts;
        polydata->GetCellPoints(cellId, nPoints, pts);
        if (nPoints != 3)
            continue;

        std::array<double, 3> pointPos[3];
        for (vtkIdType j = 0; j < nPoints; j++)
        {
            polydata->GetPoint(pts[j], pointPos[j].data());
        }
        std::array<double, 3> vec1;
		for(std::size_t i = 0; i < vec1.size(); ++i)
			vec1[i] = pointPos[1][i] - pointPos[0][i];
        std::array<double, 3> vec2;
		for(std::size_t i = 0; i < vec2.size(); ++i)
			vec2[i] = pointPos[2][i] - pointPos[1][i];
        std::array<double, 3> normal =
		{
			vec1[1]*vec2[2]-vec1[2]*vec2[1],
			vec1[2]*vec2[0]-vec1[0]*vec2[2],
			vec1[0]*vec2[1]-vec1[1]*vec2[0]
		};
        double area = vtkTriangle::TriangleArea(pointPos[0].data(), pointPos[1].data(), pointPos[2].data());
		double length = std::sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
		if(length!=0.0)
		{
			for(auto& iter :normal)
				iter = iter / length;
		}
		for(std::size_t i = 0; i < 3; ++i)
			totalNormal[i] += normal[i]*area;
    }

	{
		double length = std::sqrt(totalNormal[0]*totalNormal[0]+totalNormal[1]*totalNormal[1]+totalNormal[2]*totalNormal[2]);
		if(length !=0.0)
		{
			for(auto& iter :totalNormal)
				iter = iter / length;
		}
	}
    return totalNormal;
}

std::vector<std::array<double,3>> computeIntersectionPolygon(vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkPlane> plane)
{
	std::vector<std::array<double,3>> ret;
	auto cutter = vtkSmartPointer<vtkCutter>::New();
	cutter->SetCutFunction(plane);
	cutter->SetInputData(polydata);
	cutter->Update();

	auto intersectionData = vtkSmartPointer<vtkPolyData>::New();
	intersectionData->ShallowCopy(cutter->GetOutput());

	cleanPolydata(intersectionData);

	if(intersectionData->GetNumberOfPoints() > 0)
	{
		std::map<vtkIdType, vtkIdType> mpLine;
		for(vtkIdType i = 0; i < intersectionData->GetNumberOfCells(); ++i)
		{
			auto cell = intersectionData->GetCell(i);
			if(!mpLine.count(cell->GetPointId(0)))
				mpLine.insert(std::make_pair(cell->GetPointId(0), cell->GetPointId(1)));
			else
				mpLine.insert(std::make_pair(cell->GetPointId(1), cell->GetPointId(0)));
		}
		std::vector<vtkIdType> vertexs;
		vertexs.reserve(intersectionData->GetNumberOfPoints());
		vtkIdType firstVertex(mpLine.cbegin()->first), nextVertex(mpLine.cbegin()->first);
		do
		{
			vertexs.push_back(nextVertex);
			nextVertex = mpLine[nextVertex];
		} while(firstVertex != nextVertex);

		std::array<double,3> xyz = {0.0};
		for(auto id : vertexs)
		{
			intersectionData->GetPoint(id, xyz.data());
			ret.push_back(xyz);
		}
	}
	return ret;
}

std::vector<std::array<double,3>> polygonPoints(vtkSmartPointer<vtkPolyData> polydata)
{
	std::vector<std::array<double,3>> ret;
	vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
	pd->DeepCopy(polydata);
	cleanPolydata(pd);

	if(pd->GetNumberOfPoints() > 0)
	{
		std::map<vtkIdType, vtkIdType> mpLine;
		for(vtkIdType i = 0; i < pd->GetNumberOfCells(); ++i)
		{
			auto cell = pd->GetCell(i);
			if(!mpLine.count(cell->GetPointId(0)))
				mpLine.insert(std::make_pair(cell->GetPointId(0), cell->GetPointId(1)));
			else
				mpLine.insert(std::make_pair(cell->GetPointId(1), cell->GetPointId(0)));
		}
		std::vector<vtkIdType> vertexs;
		vertexs.reserve(pd->GetNumberOfPoints());
		vtkIdType firstVertex(mpLine.cbegin()->first), nextVertex(mpLine.cbegin()->first);
		do
		{
			vertexs.push_back(nextVertex);
			auto itpos = mpLine.find(nextVertex);
			if(itpos != mpLine.end())
				nextVertex = itpos->second;
			else
				break;
		} while(firstVertex != nextVertex);

		std::array<double,3> xyz = {0.0};
		for(auto id : vertexs)
		{
			pd->GetPoint(id, xyz.data());
			ret.push_back(xyz);
		}
	}
	return ret;
}

void printPolygon(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata)
{
	auto points = polygonPoints(polydata);
	os << "polygon points" << std::endl;
	for(const auto& iter : points)
		os << iter[0] << "," << iter[1] << "," << iter[2] << std::endl;
}

vtkSmartPointer<vtkPolyData> createCylinderData(const std::array<double, 3>& pt1, const std::array<double, 3>& pt2, float radius)
{
	auto line = vtkSmartPointer<vtkLineSource>::New();
	line->SetPoint1(pt1[0], pt1[1], pt1[2]);
	line->SetPoint2(pt2[0], pt2[1], pt2[2]);
	line->Update();
	auto tube = vtkSmartPointer<vtkTubeFilter>::New();
	tube->SetInputData(line->GetOutput());
	tube->SetRadius(radius);
	tube->Update();

	auto cylinderPd = vtkSmartPointer<vtkPolyData>::New();
	cylinderPd->DeepCopy(tube->GetOutput());
	return cylinderPd;
}

vtkSmartPointer<vtkPolyData> createPolyLineData(std::vector<std::array<double, 3>>& points)
{
	auto vtkpoints = vtkSmartPointer<vtkPoints>::New();
	for(const auto& iter : points)
		vtkpoints->InsertNextPoint(iter.data());
	
	auto polyline = vtkSmartPointer<vtkPolyLine>::New();
	polyline->GetPointIds()->SetNumberOfIds(points.size());
	for(std::size_t i = 0; i < points.size(); ++i)
		polyline->GetPointIds()->SetId(i, i);

	auto cellarray = vtkSmartPointer<vtkCellArray>::New();
	cellarray->InsertNextCell(polyline);

	auto ret = vtkSmartPointer<vtkPolyData>::New();
	ret->SetPoints(vtkpoints);
	ret->SetLines(cellarray);
	return ret;
}

vtkSmartPointer<vtkPolyData> createMultiPointsData(std::vector<std::array<double, 3>>& points, float radius)
{
    vtkSmartPointer<vtkFloatArray> scales = vtkSmartPointer<vtkFloatArray>::New();
    scales->SetName("scales");

	auto vtkpoints = vtkSmartPointer<vtkPoints>::New();
	for(const auto& iter : points)
	{
		vtkpoints->InsertNextPoint(iter.data());
		scales->InsertNextValue(radius);
	}

	vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
	grid->SetPoints(vtkpoints);
	grid->GetPointData()->AddArray(scales);    
    grid->GetPointData()->SetActiveScalars("scales"); // !!!to set radius first

	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();

	vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
	glyph3D->SetInputData(grid);
    glyph3D->SetSourceConnection(sphereSource->GetOutputPort());
	glyph3D->Update();

	auto ret = vtkSmartPointer<vtkPolyData>::New();
	ret->DeepCopy(glyph3D->GetOutput());
	return ret;
}

//vtkSmartPointer<vtkIdList> GetConnectedVertices(vtkSmartPointer<vtkPolyData> polydata, int id)
//{
//	vtkSmartPointer<vtkIdList> connectedVertices = vtkSmartPointer<vtkIdList>::New();
//
//	vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
//	polydata->GetPointCells(id, cellIdList);
//
//	for (vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
//	{
//		vtkSmartPointer<vtkIdList> pointIdList = vtkSmartPointer<vtkIdList>::New();
//		polydata->GetCellPoints(cellIdList->GetId(i), pointIdList);
//
//		if (pointIdList->GetId(0) != id)
//		{
//			connectedVertices->InsertNextId(pointIdList->GetId(0));
//		}
//		else
//		{
//			connectedVertices->InsertNextId(pointIdList->GetId(1));
//		}
//	}
//
//	return connectedVertices;
//}
//
//std::vector<std::vector<std::array<double,3>>> spiralPointsFromPolydata(vtkSmartPointer<vtkPolyData> polydata)
//{
//	std::vector<std::vector<vtkIdType>> retPointID(1);
//	// get neighbor vertex for each vertex
//	std::vector<std::vector<vtkIdType>> vecNeighborId;
//	for(vtkIdType i = 0; i < polydata->GetNumberOfPoints(); ++i)
//	{
//		vecNeighborId.push_back(std::vector<vtkIdType>());
//		auto neighbor = GetConnectedVertices(polydata, i);
//		for(vtkIdType j = 0; j < neighbor->GetNumberOfIds(); ++j)
//			vecNeighborId.back().push_back(neighbor->GetId(j));
//		std::sort(vecNeighborId.back().begin(), vecNeighborId.back().end());
//	}
//
//	auto featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
//	featureEdges->SetInputData(polydata);
//	featureEdges->BoundaryEdgesOn();
//	featureEdges->FeatureEdgesOff();
//	featureEdges->ManifoldEdgesOff();
//	featureEdges->NonManifoldEdgesOff();
//	featureEdges->ColoringOn();
//	featureEdges->Update();
//	auto edgePoints = polygonPoints(featureEdges->GetOutput());
//
//	// get vertex layer for each vertex
//	std::vector<vtkIdType> layer(polydata->GetNumberOfPoints(), 0);
//	for (vtkIdType i = 0; i < edgePoints.size(); ++i)
//	{
//		auto&& id = polydata->FindPoint(edgePoints.at(i).data());
//		layer.at(id) = 1;
//		retPointID.front().push_back(id);
//	}
//	vtkIdType curSearchLayer = 1;
//	while(std::any_of(layer.cbegin(), layer.cend(), [](vtkIdType n) {return n == 0;}))
//	{
//		std::vector<vtkIdType> curLayerPoints;
//		for (std::size_t i = 0; i < layer.size(); ++i)
//		{
//			if (layer[i] == curSearchLayer)
//				curLayerPoints.push_back(i);
//		}
//		for(const auto& iter : curLayerPoints)
//		{
//			for(const auto& pt : vecNeighborId[iter])
//			{
//				if(layer[pt] == 0)
//					layer[pt] = curSearchLayer+1;
//			}
//		}
//
//		++curSearchLayer;
//	}
//
//	retPointID.resize(curSearchLayer);
//
//	auto findNextPoint = [&](vtkIdType p1, vtkIdType p2, vtkIdType nlayer, vtkIdType& rNextId)->bool
//	{
//		bool ret(false);
//		const auto& rCurNeighbors = vecNeighborId.at(p1);
//		const auto& rLastLAyerNeighbors = vecNeighborId.at(p2);
//		std::vector<vtkIdType> commonNeighbor;
//		std::set_intersection(rCurNeighbors.cbegin(), rCurNeighbors.cend(), rLastLAyerNeighbors.cbegin(), rLastLAyerNeighbors.cend(),
//				std::back_inserter(commonNeighbor));
//		auto iter = std::find_if(commonNeighbor.cbegin(), commonNeighbor.cend(), [&](int j){return layer.at(j) == nlayer;});
//		if(iter != commonNeighbor.cend())
//		{
//			rNextId = *iter;
//			ret = true;
//		}
//		return ret;
//	};
//
//
//	vtkIdType curPointId = retPointID[0].back();
//	vtkIdType nextPointId(0);
//	for (std::size_t i = 1; i < retPointID.size(); ++i)
//	{
//		const std::size_t cntCurrenLayerPoint = std::count_if(layer.cbegin(), layer.cend(), [i](vtkIdType n){return n == (i+1);});
//		std::set<vtkIdType> currentLayerFindPoints;
//		for(const auto& lastLayerPoint : retPointID.at(i-1))
//		{
//			while(findNextPoint(lastLayerPoint, curPointId, i+1, nextPointId))
//			{
//				retPointID.at(i).push_back(nextPointId);
//				curPointId = nextPointId;
//				currentLayerFindPoints.insert(curPointId);
//				if(currentLayerFindPoints.size() == cntCurrenLayerPoint)
//					break;
//			}
//			if(currentLayerFindPoints.size() == cntCurrenLayerPoint)
//				break;
//		}
//	}
//
//	std::vector<std::vector<std::array<double,3>>> ret(retPointID.size());
//	for (const auto& nLayerPoints : retPointID)
//	{
//		for(const auto& id : nLayerPoints)
//		{
//			std::array<double,3> point;
//			polydata->GetPoint(id, point.data());
//			ret.front().push_back(point);
//		}
//	}
//	return ret;
//}
