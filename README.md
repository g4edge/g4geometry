# g4geometry
Extended geometry input and output geant4

## Build dependencies

The project now requires Open CASCADE Technology (OCCT) in addition to the existing Geant4, CGAL, Eigen3, and Catch2 dependencies. Make sure CMake can find `OpenCASCADEConfig.cmake`; if your OCCT installation is in a non-standard prefix, set `OpenCASCADE_DIR` when configuring the project.

## CAD import

`G4CADReader` now uses Open CASCADE Technology (OCCT) to load CAD files before they are converted into Geant4 geometry. The reader supports:

- STEP (`.step`, `.stp`)
- IGES (`.iges`, `.igs`)
- BREP (`.brep`)
