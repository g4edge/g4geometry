#ifndef GEANT4GEOMETRY_G4CADREADER_HH
#define GEANT4GEOMETRY_G4CADREADER_HH

#include <memory>
#include <string>

enum class G4CADFileFormat {
    Auto,
    STEP,
    IGES,
    BREP
};

class G4CADReader {
public:
    G4CADReader();
    ~G4CADReader();

    G4CADReader(const G4CADReader&) = delete;
    G4CADReader& operator=(const G4CADReader&) = delete;
    G4CADReader(G4CADReader&&) noexcept;
    G4CADReader& operator=(G4CADReader&&) noexcept;
    void Read(const std::string& filePath, G4CADFileFormat format = G4CADFileFormat::Auto);
    void Read(const std::string& filePath, G4CADFileFormat format = G4CADFileFormat::Auto);
    [[nodiscard]] bool HasShape() const;

private:
    class Impl;

    static G4CADFileFormat ResolveFormat(const std::string& filePath, G4CADFileFormat format);

    std::unique_ptr<Impl> m_impl;
};

#endif //GEANT4GEOMETRY_G4CADREADER_HH
