#pragma once

#include <string>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <Geom_Curve.hxx>
#include <TopoDS_Vertex.hxx>

struct NativeFaceData
{
    int entityLabel;
    bool sameSense;
};

struct BuildEdgeParams
{
    BRep_Builder& builder;
    ShapeFix_Edge& edgeFixer;
    TopTools_DataMapOfIntegerShape& edgeCurves;
    TopTools_DataMapOfIntegerShape& vertexGeometries; 

    double sewingTolerance;
    bool   buildRuledSurface;
};

class NativeAdvancedFaces
{
public:
   
    static TopoDS_Shape BuildAdvancedFaces(
        const std::vector<NativeFaceData>& facesData,
        double sewingTolerance,
        std::string& outLogMessage
    );

    static TopoDS_Edge BuildOrientEdge(
        int                   edgeLabel,
        bool                  orientation,
        Handle(Geom_Curve)    sharedEdgeGeom,
        const TopoDS_Vertex& startVertex,
        const TopoDS_Vertex& endVertex,
        std::string& outLog,
        BuildEdgeParams& p
    );

private:
   
    static TopoDS_Shape BuildOneAdvancedFace(
        const NativeFaceData& faceData,
        double sewingTolerance,
        std::string& outLogMessage
    );

    static bool LocatePointOnCurve(
        const Handle(Geom_Curve)& C, 
        const TopoDS_Vertex& V, 
        double tolerance, 
        double& p, 
        double& distance);


   
};

