#pragma once
#include <vtkPolyDataAlgorithm.h>
#include <array>
#include <vector>

class vtkIdList;

// *****
// This algorithm calculate all cell which one vertex is inner the given sphere.
// The method NoAppendSelection() and AppendSelection() receive a sphere by point and radius.
// There are two output for this algorithm.
// Output(0) is cell polydata the last time call NoAppendSelection() or AppendSelection().
// Output(1) is cell polydata the accumulated area call AppendSelection().
// *****
class vtkAppendableSelection : public vtkPolyDataAlgorithm
{
private:
	std::array<double, 3> m_pos3d;
	double m_dRadius;
	bool m_bAppend;
	bool m_bSetSelection;
	bool m_bClearSelection;
	vtkSmartPointer<vtkIdList> m_selectedRegion;
	vtkSmartPointer<vtkIdList> m_applyRegion;
	std::vector<std::array<double, 3>> m_selectedCeneters;
	void SelectionSetting(std::array<double, 3> pos3d, double radius, bool append);

public:
	void NoAppendSelection(std::array<double, 3> pos3d, double radius);
	void AppendSelection(std::array<double, 3> pos3d, double radius);
	void ClearSelection();
	vtkSmartPointer<vtkIdList> GetAppliedRegionIds();
	std::vector<std::array<double, 3>> SelectedCenters() const;

public:
    vtkTypeMacro(vtkAppendableSelection, vtkPolyDataAlgorithm);
    static vtkAppendableSelection* New();

protected:
    vtkAppendableSelection();
    ~vtkAppendableSelection() override = default;

    int ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

private:
    vtkAppendableSelection(const vtkAppendableSelection&) = delete;
    void operator= (const vtkAppendableSelection&) = delete;
};

