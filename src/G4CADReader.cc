#include "G4CADReader.hh"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <IGESControl_Reader.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>

namespace {

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

void EnsureReadSuccess(IFSelect_ReturnStatus status, const std::string& filePath, const char* format)
{
    if (status != IFSelect_RetDone) {
        throw std::runtime_error("Failed to read " + std::string(format) + " file: " + filePath);
    }
}

void EnsureTransferSuccess(Standard_Integer transferredRoots, const std::string& filePath, const char* format)
{
    if (transferredRoots <= 0) {
        throw std::runtime_error("Failed to translate " + std::string(format) + " geometry: " + filePath);
    }
}

}

class G4CADReader::Impl {
public:
    TopoDS_Shape shape;
};

G4CADReader::G4CADReader() : m_impl(std::make_unique<Impl>())
{
}

G4CADReader::~G4CADReader() = default;

G4CADReader::G4CADReader(G4CADReader&&) noexcept = default;

G4CADReader& G4CADReader::operator=(G4CADReader&&) noexcept = default;

G4CADFileFormat G4CADReader::ResolveFormat(const std::string& filePath, G4CADFileFormat format)
{
    if (format != G4CADFileFormat::Auto) {
        return format;
    }

    const std::string::size_type extensionStart = filePath.find_last_of('.');
    const std::string extension =
        extensionStart == std::string::npos ? std::string() : ToLower(filePath.substr(extensionStart));
    if (extension == ".stp" || extension == ".step") {
        return G4CADFileFormat::STEP;
    }
    if (extension == ".igs" || extension == ".iges") {
        return G4CADFileFormat::IGES;
    }
    if (extension == ".brep") {
        return G4CADFileFormat::BREP;
    }

    if (extension.empty()) {
        throw std::invalid_argument("Could not determine CAD format from file path: " + filePath);
    }

    throw std::invalid_argument("Unsupported CAD file extension '" + extension + "' for file: " + filePath);
}

void G4CADReader::Read(const std::string& filePath, G4CADFileFormat format)
{
    const G4CADFileFormat resolvedFormat = ResolveFormat(filePath, format);
    m_impl->shape = TopoDS_Shape();

    switch (resolvedFormat) {
        case G4CADFileFormat::STEP: {
            STEPControl_Reader reader;
            EnsureReadSuccess(reader.ReadFile(filePath.c_str()), filePath, "STEP");
            EnsureTransferSuccess(reader.TransferRoots(), filePath, "STEP");
            m_impl->shape = reader.OneShape();
            break;
        }
        case G4CADFileFormat::IGES: {
            IGESControl_Reader reader;
            EnsureReadSuccess(reader.ReadFile(filePath.c_str()), filePath, "IGES");
            EnsureTransferSuccess(reader.TransferRoots(), filePath, "IGES");
            m_impl->shape = reader.OneShape();
            break;
        }
        case G4CADFileFormat::BREP: {
            BRep_Builder builder;
            if (!BRepTools::Read(m_impl->shape, filePath.c_str(), builder)) {
                throw std::runtime_error("Failed to read BREP file: " + filePath);
            }
            break;
        }
        case G4CADFileFormat::Auto:
            throw std::logic_error("Automatic CAD format resolution did not produce a concrete format");
    }

    if (m_impl->shape.IsNull()) {
        throw std::runtime_error("Loaded CAD file did not produce a shape: " + filePath);
    }
}

bool G4CADReader::HasShape() const
{
    return !m_impl->shape.IsNull();
}

TopoDS_Shape& G4CADReader::GetShape() const
{
    return m_impl->shape;
}
