#pragma once

#include <string>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Curve.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Builder.hxx>
#include <ShapeFix_Edge.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeAnalysis.hxx>
#include <Geom_Line.hxx>
#include <BRepFill.hxx>
#include <BRepFill_Filling.hxx>
#include <BRep_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRep_Tool.hxx>
#include <BRepCheck.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Extrema_ExtPC.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include "CanLogBase.h"

struct BuildEdgeParams
{
    TopoDS_Vertex start;
    TopoDS_Vertex end;
    int edgeLabel;
    Handle(Geom_Curve) sharedEdgeGeom;
    double sewingTolerance;
    bool orientation;
};


struct BuildFaceBoundParams {
    bool isOuter;
    bool orientation;
    bool buildRuledSurface;
    std::vector<BuildEdgeParams> edges;
    double sewingTolerance;
};

struct BuildFaceParams {
    int label;
    int numberOfBounds;
    bool buildRuledSurface;
    double sewingTolerance;
    TopoDS_Face face;
    std::vector<BuildFaceBoundParams> faceBoundsParams;
};

struct BuildShellParams {
    std::vector<BuildFaceParams> facesParams;
};

class NativeAdvancedFacesBuilder : public CanLogBase
{
public:
   
     TopoDS_Shape BuildShell(BuildShellParams params);

     TopoDS_Edge BuildOrientEdge(BuildEdgeParams p, ShapeFix_Edge& edgeFixer);

     void NativeAdvancedFacesBuilder::BuildLoopWire(
        BuildFaceBoundParams           p,
        const TopoDS_Face&              topoAdvancedFace,
        TopoDS_Wire&                    topoOuterLoop,
        TopTools_SequenceOfShape&       topoInnerLoops);

     TopoDS_Face BuildFace(BuildFaceParams params, const BRep_Builder& builder);

     TopoDS_Vertex GetVertex(int label, double x, double y, double z);


private:

     TopTools_DataMapOfIntegerShape edgeCurves;
     TopTools_DataMapOfIntegerShape vertexGeometries;
     BRep_Builder builder;

     bool LocatePointOnCurve(
        const Handle(Geom_Curve)& C, 
        const TopoDS_Vertex& V, 
        double tolerance, 
        double& p, 
        double& distance);

  
     bool WithinTolerance(const  TopoDS_Wire& topoOuterLoop, const TopoDS_Face& topoAdvancedFace, double _sewingTolerance);


   
};

