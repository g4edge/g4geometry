#ifndef GEANT4GEOMETRY_G4CADEXPLORER_HH
#define GEANT4GEOMETRY_G4CADEXPLORER_HH

#include <memory>
#include <iostream>

class TopoDS_Shape;
class TopoDS_Face;
class TopoDS_Edge;


class G4CADExplorer {
public:
    G4CADExplorer();
    ~G4CADExplorer();
    void ExploreTopology(const TopoDS_Shape& shape, int depth = 0);

private:
  class Impl;

  std::unique_ptr<Impl> m_impl;

};

void PrintFaceInfo(const TopoDS_Face& face, std::ostream& os = std::cout);
void PrintEdgeInfo(const TopoDS_Edge& edge, std::ostream& os = std::cout);


#endif // GEANT4GEOMETRY_G4CADEXPLORER_HH
