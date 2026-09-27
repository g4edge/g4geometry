#include "G4CADViewer.hh"

#include <stdexcept>

#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTriangle.h>

G4CADViewer::G4CADViewer() {
    // Constructor implementation
}

G4CADViewer::~G4CADViewer() {
    // Destructor implementation
}

void G4CADViewer::DisplayShape(const TopoDS_Shape& shape, const std::string& windowTitle) const
{
  if (shape.IsNull()) {
    throw std::invalid_argument("Cannot display a null TopoDS_Shape");
  }

  BRepMesh_IncrementalMesh mesher(shape, 0.5, Standard_False, 0.5, Standard_True);
  if (!mesher.IsDone()) {
    throw std::runtime_error("Failed to mesh TopoDS_Shape for VTK display");
  }

  vtkNew<vtkPoints> points;
  vtkNew<vtkCellArray> cells;

  for (TopExp_Explorer faceExplorer(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
    const TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
    TopLoc_Location location;
    const Handle(Poly_Triangulation)& triangulation = BRep_Tool::Triangulation(face, location);
    if (triangulation.IsNull()) {
      continue;
    }

    const gp_Trsf transform = location.Transformation();
    const Poly_Array1OfTriangle& triangles = triangulation->InternalTriangles();
    const TopAbs_Orientation orientation = face.Orientation();
    const bool reverseWinding = orientation == TopAbs_REVERSED;

    for (Standard_Integer triangleIndex = triangles.Lower(); triangleIndex <= triangles.Upper(); ++triangleIndex) {
      Standard_Integer node1 = 0;
      Standard_Integer node2 = 0;
      Standard_Integer node3 = 0;
      triangles(triangleIndex).Get(node1, node2, node3);

      const gp_Pnt point1 = triangulation->Node(node1).Transformed(transform);
      const gp_Pnt point2 = triangulation->Node(node2).Transformed(transform);
      const gp_Pnt point3 = triangulation->Node(node3).Transformed(transform);

      const vtkIdType pointId1 = points->InsertNextPoint(point1.X(), point1.Y(), point1.Z());
      const vtkIdType pointId2 = points->InsertNextPoint(point2.X(), point2.Y(), point2.Z());
      const vtkIdType pointId3 = points->InsertNextPoint(point3.X(), point3.Y(), point3.Z());

      vtkNew<vtkTriangle> triangle;
      triangle->GetPointIds()->SetId(0, pointId1);
      if (reverseWinding) {
        triangle->GetPointIds()->SetId(1, pointId3);
        triangle->GetPointIds()->SetId(2, pointId2);
      } else {
        triangle->GetPointIds()->SetId(1, pointId2);
        triangle->GetPointIds()->SetId(2, pointId3);
      }
      cells->InsertNextCell(triangle);
    }
  }

  if (cells->GetNumberOfCells() == 0) {
    throw std::runtime_error("No triangulated faces available for VTK display");
  }

  vtkNew<vtkPolyData> polyData;
  polyData->SetPoints(points);
  polyData->SetPolys(cells);

  vtkNew<vtkPolyDataNormals> normals;
  normals->SetInputData(polyData);
  normals->ConsistencyOn();
  normals->SplittingOff();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(normals->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  vtkNew<vtkNamedColors> colors;
  actor->GetProperty()->SetColor(colors->GetColor3d("LightSteelBlue").GetData());
  actor->GetProperty()->SetSpecular(0.2);
  actor->GetProperty()->SetSpecularPower(20.0);

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName(windowTitle.c_str());
  renderWindow->SetSize(1200, 800);

  vtkNew<vtkRenderWindowInteractor> interactor;
  vtkNew<vtkInteractorStyleTrackballCamera> interactionStyle;
  interactor->SetInteractorStyle(interactionStyle);
  interactor->SetRenderWindow(renderWindow);

  renderer->ResetCamera();
  renderWindow->Render();
  interactor->Start();
}