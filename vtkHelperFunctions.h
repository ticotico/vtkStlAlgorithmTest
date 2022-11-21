#pragma once

#include <vector>
#include <array>
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkIdList;
class vtkPlane;

void printPolydataInformation(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata);
void printPolydataDetail(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata);
void printPolygon(std::ostream& os, vtkSmartPointer<vtkPolyData> polydata);
void cleanPolydata(vtkSmartPointer<vtkPolyData> polydata);
bool isManifold(vtkSmartPointer<vtkPolyData> polydata);
vtkSmartPointer<vtkPolyData> rebuildPolyData(vtkSmartPointer<vtkPolyData> polydata);
void computeNormals(vtkSmartPointer<vtkPolyData> polydata);

std::array<double, 3> computeSelectedCellsNormal(vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkIdList> slectRegion);
std::vector<std::array<double,3>> computeIntersectionPolygon(vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkPlane> plane);
std::vector<std::array<double,3>> polygonPoints(vtkSmartPointer<vtkPolyData> polydata);
vtkSmartPointer<vtkPolyData> createCylinderData(const std::array<double, 3>& pt1, const std::array<double, 3>& pt2, float radius);
vtkSmartPointer<vtkPolyData> createPolyLineData(std::vector<std::array<double, 3>>& points);
vtkSmartPointer<vtkPolyData> createMultiPointsData(std::vector<std::array<double, 3>>& points, float radius);
//std::vector<std::vector<std::array<double,3>>> spiralPointsFromPolydata(vtkSmartPointer<vtkPolyData> polydata);
