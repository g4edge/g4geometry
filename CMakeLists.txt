#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.21)
project(geant4geometry)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available Vis drivers by default
# You can set WITH_GEANT4_VIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_VIS "Build example with Geant4 Vis drivers" ON)
if(WITH_GEANT4_VIS)
  find_package(Geant4 REQUIRED gdml vis_all)
else()
  find_package(Geant4 REQUIRED gdml)
endif()

#----------------------------------------------------------------------------
# Add external 3rd party libraries
#

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
find_package(CGAL 5 REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(Catch2 REQUIRED)
find_package(OpenCASCADE CONFIG REQUIRED COMPONENTS FoundationClasses ModelingData DataExchange)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB CADSources ${PROJECT_SOURCE_DIR}/src/G4CAD*cc)
file(GLOB CADHeaders ${PROJECT_SOURCE_DIR}/include/G4CAD*.hh)
add_library(g4cad SHARED ${CADSources} ${CADHeaders})
target_compile_options(g4cad PRIVATE -Werror )
target_include_directories(g4cad
                           PUBLIC ${PROJECT_SOURCE_DIR}/include
                           PUBLIC ${OpenCASCADE_INCLUDE_DIR}
                           PRIVATE ${Geant4_INCLUDE_DIR})
target_link_libraries(g4cad PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(g4cad PUBLIC ${OpenCASCADE_LIBRARIES})

#file(GLOB CADTestSources ${PROJECT_SOURCE_DIR}/test_cad/*cc)
#file(GLOB CADTestHeaders ${PROJECT_SOURCE_DIR}/test_cad/*hh)
#add_executable(cad_test ${CADTestSources} ${CADTestHeaders})
#target_compile_options(cad_test PRIVATE -Werror)
#target_link_libraries(cad_test ${Geant4_LIBRARIES} g4halfspace)
