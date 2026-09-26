#ifndef GEANT4GEOMETRY_G4CADREADER_HH
#define GEANT4GEOMETRY_G4CADREADER_HH

#include <string>

#include <TopoDS_Shape.hxx>

enum class G4CADFileFormat {
    Auto,
    STEP,
    IGES,
    BREP
};

class G4CADReader {
public:
    G4CADReader() = default;
    virtual ~G4CADReader() = default;

    void Read(const std::string& filePath, G4CADFileFormat format = G4CADFileFormat::Auto);
    [[nodiscard]] const TopoDS_Shape& GetShape() const;
    [[nodiscard]] bool HasShape() const;

private:
    static G4CADFileFormat ResolveFormat(const std::string& filePath, G4CADFileFormat format);

    TopoDS_Shape m_shape;
};

#endif //GEANT4GEOMETRY_G4CADREADER_HH
