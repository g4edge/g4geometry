#include "G4CADExplorer.hh"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Iterator.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Vertex.hxx"
#include "TopoDS.hxx"
#include "TopExp.hxx"
#include "TopExp_Explorer.hxx"
#include "GeomAbs_SurfaceType.hxx"
#include "BRep_Tool.hxx"
#include "BRepAdaptor_Surface.hxx"
#include "BRepAdaptor_Curve.hxx"
#include "BRepGProp.hxx"
#include "BRepTools.hxx"
#include <GProp_GProps.hxx>
#include <BRepBndLib.hxx>

class G4CADExplorer::Impl {
public:
};

static std::string ShapeTypeName(TopAbs_ShapeEnum type)
{
  switch (type) {
    case TopAbs_COMPOUND:  return "COMPOUND";
    case TopAbs_COMPSOLID: return "COMPSOLID";
    case TopAbs_SOLID:     return "SOLID";
    case TopAbs_SHELL:     return "SHELL";
    case TopAbs_FACE:      return "FACE";
    case TopAbs_WIRE:      return "WIRE";
    case TopAbs_EDGE:      return "EDGE";
    case TopAbs_VERTEX:    return "VERTEX";
    default:               return "SHAPE";
  }
}

static std::string SurfaceTypeName(GeomAbs_SurfaceType t)
{
  switch (t) {
    case GeomAbs_Plane:            return "Plane";
    case GeomAbs_Cylinder:         return "Cylinder";
    case GeomAbs_Cone:             return "Cone";
    case GeomAbs_Sphere:           return "Sphere";
    case GeomAbs_Torus:            return "Torus";
    case GeomAbs_BezierSurface:    return "BezierSurface";
    case GeomAbs_BSplineSurface:   return "BSplineSurface";
    case GeomAbs_SurfaceOfRevolution: return "SurfaceOfRevolution";
    case GeomAbs_SurfaceOfExtrusion:  return "SurfaceOfExtrusion";
    case GeomAbs_OffsetSurface:    return "OffsetSurface";
    default:                       return "OtherSurface";
  }
}

static std::string CurveTypeName(GeomAbs_CurveType t)
{
  switch (t) {
    case GeomAbs_Line:            return "Line";
    case GeomAbs_Circle:          return "Circle";
    case GeomAbs_Ellipse:         return "Ellipse";
    case GeomAbs_Hyperbola:       return "Hyperbola";
    case GeomAbs_Parabola:        return "Parabola";
    case GeomAbs_BezierCurve:     return "BezierCurve";
    case GeomAbs_BSplineCurve:    return "BSplineCurve";
    case GeomAbs_OffsetCurve:     return "OffsetCurve";
    default:                      return "OtherCurve";
  }
}

static std::string OrientationName(TopAbs_Orientation o)
{
  switch (o) {
    case TopAbs_FORWARD:  return "FORWARD";
    case TopAbs_REVERSED: return "REVERSED";
    case TopAbs_INTERNAL: return "INTERNAL";
    case TopAbs_EXTERNAL: return "EXTERNAL";
    default:              return "UNKNOWN";
  }
}

G4CADExplorer::G4CADExplorer() : m_impl(std::make_unique<Impl>()) {

}

G4CADExplorer::~G4CADExplorer() = default;

void G4CADExplorer::ExploreTopology(const TopoDS_Shape& shape, int depth)
{
  std::string indent(depth * 2, ' ');
  std::cout << indent << ShapeTypeName(shape.ShapeType())
            << "  [orientation="
            << (shape.Orientation() == TopAbs_FORWARD ? "FWD" :
                shape.Orientation() == TopAbs_REVERSED ? "REV" : "OTHER")
            << "]\n";

  it.Value().ShapeType() == TopAbs_FACE ? PrintFaceInfo(TopoDS::Face(it.Value())) : void();
  it.Value().ShapeType() == TopAbs_EDGE ? PrintEdgeInfo(TopoDS::Edge(it.Value())) : void();

  // TopoDS_Iterator walks immediate children only (one level down),
  // respecting the natural containment hierarchy (compound->solid->shell->face->wire->edge->vertex)
  for (TopoDS_Iterator it(shape); it.More(); it.Next()) {
    ExploreTopology(it.Value(), depth + 1);

  }
}

void PrintFaceInfo(const TopoDS_Face& face, std::ostream& os)
{
  os << std::fixed << std::setprecision(6);
  os << "----- Face Info -----\n";

  // Orientation & tolerance
  os << "Orientation      : " << OrientationName(face.Orientation()) << "\n";
  os << "Tolerance        : " << BRep_Tool::Tolerance(face) << "\n";
  os << "Is NaturalRestr. : " << (BRep_Tool::NaturalRestriction(face) ? "yes" : "no") << "\n";

  // Underlying surface via adaptor (respects trims/parametrization)
  BRepAdaptor_Surface adaptor(face, true);
  GeomAbs_SurfaceType type = adaptor.GetType();
  os << "Surface type     : " << SurfaceTypeName(type) << "\n";

  // Parametric range (u,v)
  os << "U range          : [" << adaptor.FirstUParameter() << ", " << adaptor.LastUParameter() << "]\n";
  os << "V range          : [" << adaptor.FirstVParameter() << ", " << adaptor.LastVParameter() << "]\n";
  os << "Is U periodic    : " << (adaptor.IsUPeriodic() ? "yes" : "no") << "\n";
  os << "Is V periodic    : " << (adaptor.IsVPeriodic() ? "yes" : "no") << "\n";

  // Surface-specific geometric parameters
  switch (type) {
    case GeomAbs_Plane: {
      gp_Pln pln = adaptor.Plane();
      gp_Pnt loc = pln.Location();
      gp_Dir n = pln.Axis().Direction();
      os << "Plane origin     : (" << loc.X() << ", " << loc.Y() << ", " << loc.Z() << ")\n";
      os << "Plane normal     : (" << n.X() << ", " << n.Y() << ", " << n.Z() << ")\n";
      break;
    }
    case GeomAbs_Cylinder: {
      gp_Cylinder cyl = adaptor.Cylinder();
      os << "Cylinder radius  : " << cyl.Radius() << "\n";
      gp_Pnt loc = cyl.Location();
      os << "Cylinder axis pt : (" << loc.X() << ", " << loc.Y() << ", " << loc.Z() << ")\n";
      break;
    }
    case GeomAbs_Cone: {
      gp_Cone cone = adaptor.Cone();
      os << "Cone semi-angle  : " << cone.SemiAngle() << " rad\n";
      os << "Cone ref radius  : " << cone.RefRadius() << "\n";
      break;
    }
    case GeomAbs_Sphere: {
      gp_Sphere sph = adaptor.Sphere();
      os << "Sphere radius    : " << sph.Radius() << "\n";
      break;
    }
    case GeomAbs_Torus: {
      gp_Torus tor = adaptor.Torus();
      os << "Torus major rad. : " << tor.MajorRadius() << "\n";
      os << "Torus minor rad. : " << tor.MinorRadius() << "\n";
      break;
    }
    case GeomAbs_BSplineSurface: {
      os << "BSpline U degree : " << adaptor.UDegree() << "\n";
      os << "BSpline V degree : " << adaptor.VDegree() << "\n";
      os << "BSpline #U poles : " << adaptor.NbUPoles() << "\n";
      os << "BSpline #V poles : " << adaptor.NbVPoles() << "\n";
      break;
    }
    default:
      break; // no extra params printed for other types
  }

  // Area (surface property integration)
  GProp_GProps surfaceProps;
  BRepGProp::SurfaceProperties(face, surfaceProps);
  os << "Area             : " << surfaceProps.Mass() << "\n";
  gp_Pnt centroid = surfaceProps.CentreOfMass();
  os << "Centroid         : (" << centroid.X() << ", " << centroid.Y() << ", " << centroid.Z() << ")\n";

  // Bounding box
  Bnd_Box bbox;
  BRepBndLib::Add(face, bbox);
  if (!bbox.IsVoid()) {
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    os << "Bounding box min : (" << xmin << ", " << ymin << ", " << zmin << ")\n";
    os << "Bounding box max : (" << xmax << ", " << ymax << ", " << zmax << ")\n";
  }

  // Wires and edges belonging to this face
  int wireCount = 0, edgeCount = 0;
  for (TopExp_Explorer wExp(face, TopAbs_WIRE); wExp.More(); wExp.Next()) {
    ++wireCount;
  }
  for (TopExp_Explorer eExp(face, TopAbs_EDGE); eExp.More(); eExp.Next()) {
    ++edgeCount;
  }
  os << "Number of wires  : " << wireCount << "\n";
  os << "Number of edges  : " << edgeCount << "\n";

  // Identify outer wire vs inner (hole) wires
  TopoDS_Wire outer = BRepTools::OuterWire(face);
  int innerWires = wireCount - (outer.IsNull() ? 0 : 1);
  os << "Inner (hole) wires: " << innerWires << "\n";

  os << "----------------------\n";
}

void PrintEdgeInfo(const TopoDS_Edge& edge, std::ostream& os)
{
  os << std::fixed << std::setprecision(6);
  os << "----- Edge Info -----\n";

  // Orientation & tolerance
  os << "Orientation      : " << OrientationName(edge.Orientation()) << "\n";
  os << "Tolerance        : " << BRep_Tool::Tolerance(edge) << "\n";
  os << "Is degenerated   : " << (BRep_Tool::Degenerated(edge) ? "yes" : "no") << "\n";
  os << "Is closed        : " << (BRep_Tool::IsClosed(edge) ? "yes" : "no") << "\n";
  os << "Is seam (same-param check skipped; see notes)\n";

  if (BRep_Tool::Degenerated(edge)) {
    os << "----------------------\n";
    return; // degenerate edges (e.g. sphere poles) have no meaningful curve
  }

  // Underlying curve via adaptor
  BRepAdaptor_Curve adaptor(edge);
  GeomAbs_CurveType type = adaptor.GetType();
  os << "Curve type       : " << CurveTypeName(type) << "\n";

  Standard_Real first = adaptor.FirstParameter();
  Standard_Real last  = adaptor.LastParameter();
  os << "Parameter range  : [" << first << ", " << last << "]\n";
  os << "Is periodic      : " << (adaptor.IsPeriodic() ? "yes" : "no") << "\n";

  // Curve-specific geometric parameters
  switch (type) {
    case GeomAbs_Line: {
      gp_Lin line = adaptor.Line();
      gp_Pnt loc = line.Location();
      gp_Dir dir = line.Direction();
      os << "Line origin      : (" << loc.X() << ", " << loc.Y() << ", " << loc.Z() << ")\n";
      os << "Line direction   : (" << dir.X() << ", " << dir.Y() << ", " << dir.Z() << ")\n";
      break;
    }
    case GeomAbs_Circle: {
      gp_Circ circ = adaptor.Circle();
      gp_Pnt center = circ.Location();
      os << "Circle radius    : " << circ.Radius() << "\n";
      os << "Circle center    : (" << center.X() << ", " << center.Y() << ", " << center.Z() << ")\n";
      break;
    }
    case GeomAbs_Ellipse: {
      gp_Elips el = adaptor.Ellipse();
      os << "Ellipse majorRad : " << el.MajorRadius() << "\n";
      os << "Ellipse minorRad : " << el.MinorRadius() << "\n";
      break;
    }
    case GeomAbs_BSplineCurve: {
      os << "BSpline degree   : " << adaptor.Degree() << "\n";
      os << "BSpline #poles   : " << adaptor.NbPoles() << "\n";
      os << "BSpline #knots   : " << adaptor.NbKnots() << "\n";
      break;
    }
    default:
      break;
  }

  // Endpoints (respecting orientation)
  gp_Pnt pFirst = adaptor.Value(first);
  gp_Pnt pLast  = adaptor.Value(last);
  os << "Start point      : (" << pFirst.X() << ", " << pFirst.Y() << ", " << pFirst.Z() << ")\n";
  os << "End point        : (" << pLast.X() << ", " << pLast.Y() << ", " << pLast.Z() << ")\n";

  // Tangent at midpoint
  Standard_Real mid = 0.5 * (first + last);
  gp_Pnt pMid;
  gp_Vec tangent;
  adaptor.D1(mid, pMid, tangent);
  os << "Midpoint         : (" << pMid.X() << ", " << pMid.Y() << ", " << pMid.Z() << ")\n";
  os << "Tangent @ mid    : (" << tangent.X() << ", " << tangent.Y() << ", " << tangent.Z() << ")\n";

  // Length (arc length integration)
  GProp_GProps lineProps;
  BRepGProp::LinearProperties(edge, lineProps);
  os << "Length           : " << lineProps.Mass() << "\n";

  // Vertices attached to this edge
  TopoDS_Vertex vFirst, vLast;
  TopExp::Vertices(edge, vFirst, vLast); // respects edge orientation
  if (!vFirst.IsNull()) {
    gp_Pnt vp = BRep_Tool::Pnt(vFirst);
    os << "Vertex (first)   : (" << vp.X() << ", " << vp.Y() << ", " << vp.Z()
       << ")  tol=" << BRep_Tool::Tolerance(vFirst) << "\n";
  }
  if (!vLast.IsNull()) {
    gp_Pnt vp = BRep_Tool::Pnt(vLast);
    os << "Vertex (last)    : (" << vp.X() << ", " << vp.Y() << ", " << vp.Z()
       << ")  tol=" << BRep_Tool::Tolerance(vLast) << "\n";
  }

  // Bounding box
  Bnd_Box bbox;
  BRepBndLib::Add(edge, bbox);
  if (!bbox.IsVoid()) {
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    os << "Bounding box min : (" << xmin << ", " << ymin << ", " << zmin << ")\n";
    os << "Bounding box max : (" << xmax << ", " << ymax << ", " << zmax << ")\n";
  }

  os << "----------------------\n";
}
