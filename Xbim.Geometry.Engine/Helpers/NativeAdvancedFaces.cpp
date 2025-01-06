#include "NativeAdvancedFaces.h"

// --- OpenCascade includes ---
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


TopoDS_Shape NativeAdvancedFaces::BuildAdvancedFaces(
    const std::vector<NativeFaceData>& facesData,
    double sewingTolerance,
    std::string& outLogMessage
)
{
    BRep_Builder builder;
    TopoDS_Shell shell;
    builder.MakeShell(shell);

    try
    {
        // Loop over each face to build
        for (auto& faceData : facesData)
        {
            // Build a single advanced face shape
            TopoDS_Shape faceShape = BuildOneAdvancedFace(faceData, sewingTolerance, outLogMessage);
            if (!faceShape.IsNull())
            {
                // Add the face to the shell
                builder.Add(shell, faceShape);
            }
        }

        // Now check & fix shell orientation
        BRepCheck_Shell checker(shell);
        BRepCheck_Status st = checker.Orientation();
        if (st != BRepCheck_Status::BRepCheck_NoError)
        {
            // Attempt to fix shell if needed
            std::string errMsg;
            // Example: if you have a custom fix
            // if (!XbimNativeApi::FixShell(shell, 10, errMsg)) {
            //     outLogMessage += "Failed to fix shell: " + errMsg + "\n";
            // }

            checker.Init(shell);
            if (checker.Orientation() == BRepCheck_Status::BRepCheck_NoError)
            {
                shell.Closed(true);
                shell.Checked(true);
                return shell;
            }
            else
            {
                // If still invalid, do a broader fix
                // if (!XbimNativeApi::FixShape(...))
                // {
                //     outLogMessage += "Failed to fix shape further.\n";
                // }
                return shell; // Return the best we have
            }
        }
        else
        {
            // Shell is oriented/closed
            shell.Closed(true);
            shell.Checked(true);
            return shell;
        }
    }
    catch (const Standard_Failure& exc)
    {
        // Log exception in outLogMessage
        outLogMessage += "General failure building advanced faces: ";
        outLogMessage += exc.GetMessageString();
        outLogMessage += "\n";
        return shell; // Return partial shell
    }
}

 
TopoDS_Shape NativeAdvancedFaces::BuildOneAdvancedFace(
    const NativeFaceData& faceData,
    double sewingTolerance,
    std::string& outLogMessage
)
{
    // Here is where you replicate the logic that builds a single face:
    // 1. Build topological data (vertex, edges, wire)
    // 2. Create the face with BRepBuilderAPI_MakeFace
    // 3. Possibly fix the face with ShapeFix_Face
    // 4. Return the resulting TopoDS_Face

    // This is just a skeleton that you can fill with your original code.
    // For brevity, we won't paste the entire method from your question,
    // but the steps are analogous:
    //
    // - Build edges from faceData
    // - Build loops (outer/inner)
    // - If necessary, build a ruled surface or use BRepFill_Filling
    // - Add wires to the face
    // - Check and fix the face
    // - Return the final TopoDS_Face

    // Example pseudocode:

    BRepBuilderAPI_MakeFace faceMaker;
    TopoDS_Face topoAdvancedFace;

    // 1) Suppose we create or retrieve the geometry for the face surface
    //    (In your real code, you'd use XbimCurve, etc.)
    //    For now, let's do something minimal:
    // gp_Pnt origin(0, 0, 0);
    // Handle(Geom_Plane) plane = new Geom_Plane(gp_Ax3(origin, gp_Dir(0, 0, 1)));
    // faceMaker.Init(plane, 0.0, 1.0, 0.0, 1.0); // Example bounding box

    // 2) If "sameSense" is false, reverse the face, etc.
    // if (!faceData.sameSense)
    // {
    //     topoAdvancedFace.Reverse();
    // }

    // 3) Build edges & wires for outer loop, inner loops, etc. 
    //    Then faceMaker.Add(outerWire); faceMaker.Add(innerWire)...

    // 4) Finish, fix, and return
    topoAdvancedFace = faceMaker.Face();
    if (topoAdvancedFace.IsNull())
    {
        outLogMessage += "Failed to create face for label: " + std::to_string(faceData.entityLabel) + "\n";
        return TopoDS_Shape();
    }

    // Example: fix orientation
    try
    {
        BRepCheck_Analyzer analyzer(topoAdvancedFace, Standard_False);
        if (!analyzer.IsValid())
        {
            ShapeFix_Shape sfs(topoAdvancedFace);
            if (sfs.Perform())
            {
                topoAdvancedFace = TopoDS::Face(sfs.Shape());
                topoAdvancedFace.Checked(true);
            }
        }
        else
        {
            topoAdvancedFace.Checked(true);
        }
    }
    catch (const Standard_Failure& sf)
    {
        outLogMessage += "Fixing single face failed: ";
        outLogMessage += sf.GetMessageString();
        outLogMessage += "\n";
    }

    return topoAdvancedFace;
}

TopoDS_Edge NativeAdvancedFaces::BuildOrientEdge(
    int                   edgeLabel,
    bool                  orientation,
    Handle(Geom_Curve)    sharedEdgeGeom,
    const TopoDS_Vertex& startVertex,
    const TopoDS_Vertex& endVertex,
    std::string& outLog,
    BuildEdgeParams& p
)
{
    // Check if we've already built this edge
    if (p.edgeCurves.IsBound(edgeLabel))
    {
        // -- Edge was already created, so we just handle orientation or reversal
        TopoDS_Edge existingEdge = TopoDS::Edge(p.edgeCurves.Find(edgeLabel));
        if (!orientation) {
            return TopoDS::Edge(existingEdge.Reversed());
        }
        else {
            return existingEdge;
        }
    }

   
    TopoDS_Edge topoEdgeCurve;

    // Example: check if the curve or vertices are valid (in real code do more robust checks).
    if (sharedEdgeGeom.IsNull()) {
        std::stringstream ss;
        ss << "BuildOrientEdge: Null geometry for edge #" << edgeLabel << ".\n";
        outLog += ss.str();
        return TopoDS_Edge(); // null
    }

    // If the curve is closed and start/end vertices are the same,
    // then we can build the entire closed edge from first to last param
    if (sharedEdgeGeom->IsClosed() && startVertex.IsSame(endVertex))
    {
        double f = sharedEdgeGeom->FirstParameter();
        double l = sharedEdgeGeom->LastParameter();
        topoEdgeCurve = BRepBuilderAPI_MakeEdge(sharedEdgeGeom, startVertex, endVertex, f, l);
    }
    else
    {
        // We may need to locate param near the start/end vertices:
        //   For instance, if you had a "LocatePointOnCurve" function in native code,
        //   you'd call it here. For demonstration, let's do a skeleton approach:

        // These would be filled from a real "LocatePointOnCurve" or some projection logic
        double trimParam1 = sharedEdgeGeom->FirstParameter();
        double trimParam2 = sharedEdgeGeom->LastParameter();
        double trim1Tolerance = p.sewingTolerance;
        double trim2Tolerance = p.sewingTolerance;

        // In your original code, you had:
            bool foundP1 = LocatePointOnCurve(sharedEdgeGeom, startVertex, p.sewingTolerance * 20, trimParam1, trim1Tolerance);
            bool foundP2 = LocatePointOnCurve(sharedEdgeGeom, startVertex, p.sewingTolerance * 20, trimParam1, trim1Tolerance);
        //    if not found => assume start or end param
        //    etc.
        // Here, replicate that logic or call a native function that does it.

        // Now update vertex tolerances if needed:
        double currentStartTol = BRep_Tool::Tolerance(startVertex);
        double currentEndTol = BRep_Tool::Tolerance(endVertex);

        if (trim1Tolerance > currentStartTol)
            p.builder.UpdateVertex(startVertex, trim1Tolerance);
        if (trim2Tolerance > currentEndTol)
            p.builder.UpdateVertex(endVertex, trim2Tolerance);

        // Make the edge:
        BRepBuilderAPI_MakeEdge edgeMaker(sharedEdgeGeom, startVertex, endVertex, trimParam1, trimParam2);
        if (!edgeMaker.IsDone())
        {
            BRepBuilderAPI_EdgeError err = edgeMaker.Error();
            std::stringstream ss;
            ss << "BuildOrientEdge: Failed to create edge #" << edgeLabel << ": ";

            switch (err)
            {
            case BRepBuilderAPI_PointProjectionFailed:
                ss << "BRepBuilderAPI_PointProjectionFailed";
                break;
            case BRepBuilderAPI_ParameterOutOfRange:
                ss << "BRepBuilderAPI_ParameterOutOfRange";
                break;
            case BRepBuilderAPI_DifferentPointsOnClosedCurve:
                ss << "BRepBuilderAPI_DifferentPointsOnClosedCurve";
                break;
            case BRepBuilderAPI_PointWithInfiniteParameter:
                ss << "BRepBuilderAPI_PointWithInfiniteParameter";
                break;
            case BRepBuilderAPI_DifferentsPointAndParameter:
                ss << "BRepBuilderAPI_DifferentsPointAndParameter";
                break;
            case BRepBuilderAPI_LineThroughIdenticPoints:
                ss << "BRepBuilderAPI_LineThroughIdenticPoints";
                break;
            default:
                ss << "Unknown error";
                break;
            }
            ss << "\n";
            outLog += ss.str();
            return TopoDS_Edge();
        }

        topoEdgeCurve = edgeMaker.Edge();
        // Possibly fix vertex tolerances with the shape fixer
        p.edgeFixer.FixVertexTolerance(topoEdgeCurve);
    }

    // Now store the newly created edge in edgeCurves
    p.edgeCurves.Bind(edgeLabel, topoEdgeCurve);

    // Reverse the edge if orientation is false
    if (!orientation) {
        topoEdgeCurve = TopoDS::Edge(topoEdgeCurve.Reversed());
    }

    return topoEdgeCurve;
}


bool NativeAdvancedFaces::LocatePointOnCurve(const Handle(Geom_Curve)& C, const TopoDS_Vertex& V, double tolerance, double& p, double& distance)
{
    Standard_Real Eps2 = tolerance * tolerance;

    gp_Pnt P = BRep_Tool::Pnt(V);
    GeomAdaptor_Curve GAC(C);

    // Afin de faire les extremas, on verifie les distances en bout
    Standard_Real D1, D2;
    gp_Pnt P1, P2;
    P1 = GAC.Value(GAC.FirstParameter());
    P2 = GAC.Value(GAC.LastParameter());
    D1 = P1.SquareDistance(P);
    D2 = P2.SquareDistance(P);
    if ((D1 < D2) && (D1 <= Eps2)) {
        p = GAC.FirstParameter();
        distance = sqrt(D1);
        return Standard_True;
    }
    else if ((D2 < D1) && (D2 <= Eps2)) {
        p = GAC.LastParameter();
        distance = sqrt(D2);
        return Standard_True;
    }

    Extrema_ExtPC extrema(P, GAC);
    if (extrema.IsDone()) {
        Standard_Integer i, index = 0, n = extrema.NbExt();
        Standard_Real Dist2 = RealLast(), dist2min;

        for (i = 1; i <= n; i++) {
            dist2min = extrema.SquareDistance(i);
            if (dist2min < Dist2) {
                index = i;
                Dist2 = dist2min;
            }
        }

        if (index != 0) {
            if (Dist2 <= Eps2) {
                p = (extrema.Point(index)).Parameter();
                distance = sqrt(Dist2);
                return Standard_True;
            }
        }
    }
    return Standard_False;
}
