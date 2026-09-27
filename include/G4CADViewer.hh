#ifndef GEANT4GEOMETRY_G4CADVIEWER_HH
#define GEANT4GEOMETRY_G4CADVIEWER_HH

#include <string>

class TopoDS_Shape;


class G4CADViewer {
public:
    G4CADViewer();
    ~G4CADViewer();

  void DisplayShape(const TopoDS_Shape& shape, const std::string& windowTitle = "G4 CAD VTK Viewer") const;
};

#endif // GEANT4GEOMETRY_G4CADVIEWER_HH