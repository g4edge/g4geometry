#include "G4CADReader.hh"
#include "G4CADExplorer.hh"
#include "G4CADViewer.hh"

#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cout << "Usage: " << argv[0] << " <CAD file path>" << std::endl;
      return 1; // No file path provided
    }

    G4CADReader reader;
    reader.Read(argv[1], G4CADFileFormat::STEP);
    if (reader.HasShape()) {
      std::cout << "Successfully loaded the shape from " << argv[1] << std::endl;
    }

    G4CADExplorer explorer;
    explorer.ExploreTopology(reader.GetShape());

    G4CADViewer viewer;
    viewer.DisplayShape(reader.GetShape());

    return 0;
}