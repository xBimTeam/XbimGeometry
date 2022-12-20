#include "NFaceMeshIterator.h"
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>

#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Graphic3d_Vec3.hxx>

#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_IndexedDataMapOfShapeStyle.hxx>
#include <XCAFPrs.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

NFaceMeshIterator::NFaceMeshIterator(
	const TDF_Label& theLabel,
	const XCAFPrs_Style& theStyle,
	const TopLoc_Location& location,
	double linearDeflection,
	double angularDeflection,
	bool checkEdges) :
	myCheckEdges(checkEdges),
	mySLTool(1, 1e-12),
	myNodes(NULL),
	myNormals(NULL),
	myHasNormals(false),
	myIsMirrored(false),
	myDefaultStyle(theStyle),
	myHasFaceColor(false),
	myAngularDeflection(angularDeflection),
	myLinearDeflection(linearDeflection)
{
	TopoDS_Shape aShape;
	if (!XCAFDoc_ShapeTool::GetShape(theLabel, aShape)
		|| aShape.IsNull())
	{
		return;
	}
	myShape = aShape;
	BRepMesh_IncrementalMesh aMesher(myShape, myLinearDeflection, false, myAngularDeflection);

	myShape.Location(location);
	myFaceIter.Init(myShape, TopAbs_FACE);
	dispatchStyles(theLabel, location, theStyle);
	Next();
}

NFaceMeshIterator::NFaceMeshIterator(const TopoDS_Shape& aShape, bool checkEdges) :
	myCheckEdges(checkEdges),
	mySLTool(1, 1e-12),
	myNodes(NULL),
	myNormals(NULL),
	myHasNormals(false),
	myIsMirrored(false)
{
	myFaceIter.Init(aShape, TopAbs_FACE);
	Next();
}


// =======================================================================
// function : Next
// purpose  :
// =======================================================================
void NFaceMeshIterator::Next()
{
	for (; myFaceIter.More(); myFaceIter.Next())
	{
		myFace = TopoDS::Face(myFaceIter.Current());
		myPolyTriang = BRep_Tool::Triangulation(myFace, myFaceLocation);
		myTrsf = myFaceLocation.Transformation();
		if (myPolyTriang.IsNull()
			|| myPolyTriang->NbTriangles() == 0)
		{
			resetFace();
			continue;
		}
		initFace();
		myFaceIter.Next();
		return;
	}

	resetFace();
}


// =======================================================================
// function : initFace
// purpose  :
// =======================================================================
void NFaceMeshIterator::initFace()
{
	myHasNormals = false;
	myIsMirrored = myTrsf.VectorialPart().Determinant() < 0.0;
	myNormals = NULL;
	if (myCheckEdges) HasCurves = hasCurves();
	//myNodes = &myPolyTriang->Nodes();
	/*if (!myPolyTriang->HasNormals())
	{
		Poly::ComputeNormals(myPolyTriang);
	}
	myNormals = &myPolyTriang->Normals();
	myHasNormals = true;*/
	if (myPolyTriang->HasUVNodes() && !myHasNormals)
	{

		TopoDS_Face aFaceFwd = TopoDS::Face(myFace.Oriented(TopAbs_FORWARD));
		aFaceFwd.Location(TopLoc_Location());
		TopLoc_Location aLoc;
		if (!BRep_Tool::Surface(aFaceFwd, aLoc).IsNull())
		{
			myFaceAdaptor.Initialize(aFaceFwd, false);
			mySLTool.SetSurface(myFaceAdaptor);
			myHasNormals = true;
		}

	}
	if (!myStyles.Find(myFace, myFaceStyle))
	{
		myFaceStyle = myDefaultStyle;
	}

	if (!myFaceStyle.Material().IsNull())
	{
		myHasFaceColor = true;
		myFaceColor = myFaceStyle.Material()->BaseColor();
	}
	else if (myFaceStyle.IsSetColorSurf())
	{
		myHasFaceColor = true;
		myFaceColor = myFaceStyle.GetColorSurfRGBA();
	}
}


// =======================================================================
// function : normal
// purpose  :
// =======================================================================
gp_Dir NFaceMeshIterator::normal(Standard_Integer theNode)
{
	gp_Dir aNormal(gp::DZ());
	if (myNormals != NULL)
	{
		const Standard_Integer aNodeIndex = theNode - myNodes->Lower();
		const Graphic3d_Vec3 aNormVec3(myNormals->Value(myNormals->Lower() + aNodeIndex * 3),
			myNormals->Value(myNormals->Lower() + aNodeIndex * 3 + 1),
			myNormals->Value(myNormals->Lower() + aNodeIndex * 3 + 2));
		if (aNormVec3.Modulus() != 0.0f)
		{
			aNormal.SetCoord(aNormVec3.x(), aNormVec3.y(), aNormVec3.z());
		}
	}
	else if (myHasNormals
		&& myPolyTriang->HasUVNodes())
	{
		const gp_XY& anUV = myPolyTriang->UVNode(theNode).XY();
		mySLTool.SetParameters(anUV.X(), anUV.Y());
		if (mySLTool.IsNormalDefined())
		{
			aNormal = mySLTool.Normal();
		}
	}
	return aNormal;
}

bool NFaceMeshIterator::hasCurves()
{
	for (TopExp_Explorer edgeExplorer(Face(), TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next())
	{
		Standard_Real start, end;
		Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(edgeExplorer.Current()), start, end);
		if (!c3d.IsNull())
		{
			if (c3d->DynamicType() == STANDARD_TYPE(Geom_Line)) //if it is a line all is well skip to next edge
				continue;

			if (c3d->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) //if it is a trimmed curve determine if basis curve is a line
			{
				//it must be a trimmed curve
				Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
				//flatten any trimeed curve nesting
				while (tc->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
					tc = Handle(Geom_TrimmedCurve)::DownCast(tc->BasisCurve());
				//get the type of the basis curve
				Handle(Standard_Type) tcType = tc->BasisCurve()->DynamicType();
				//if its a line all is well skip to next edge
				if (tcType == STANDARD_TYPE(Geom_Line))
					continue;
			}
			//if here then the shape has curves variable mesh resolution is possible
			return true;
		}
	}
	return false;
}


// =======================================================================
// function : dispatchStyles
// purpose  :
// =======================================================================
void NFaceMeshIterator::dispatchStyles(const TDF_Label& theLabel,
	const TopLoc_Location& theLocation,
	const XCAFPrs_Style& theStyle)
{
	TopLoc_Location aDummyLoc;
	XCAFPrs_IndexedDataMapOfShapeStyle aStyles;
	XCAFPrs::CollectStyleSettings(theLabel, aDummyLoc, aStyles);

	Standard_Integer aNbTypes[TopAbs_SHAPE] = {};
	for (Standard_Integer aTypeIter = TopAbs_FACE; aTypeIter >= TopAbs_COMPOUND; --aTypeIter)
	{
		if (aTypeIter != TopAbs_FACE
			&& aNbTypes[aTypeIter] == 0)
		{
			continue;
		}

		for (XCAFPrs_IndexedDataMapOfShapeStyle::Iterator aStyleIter(aStyles); aStyleIter.More(); aStyleIter.Next())
		{
			const TopoDS_Shape& aKeyShape = aStyleIter.Key();
			const TopAbs_ShapeEnum aKeyShapeType = aKeyShape.ShapeType();
			if (aTypeIter == TopAbs_FACE)
			{
				++aNbTypes[aKeyShapeType];
			}
			if (aTypeIter != aKeyShapeType)
			{
				continue;
			}

			XCAFPrs_Style aCafStyle = aStyleIter.Value();
			if (!aCafStyle.IsSetColorCurv()
				&& theStyle.IsSetColorCurv())
			{
				aCafStyle.SetColorCurv(theStyle.GetColorCurv());
			}
			if (!aCafStyle.IsSetColorSurf()
				&& theStyle.IsSetColorSurf())
			{
				aCafStyle.SetColorSurf(theStyle.GetColorSurfRGBA());
			}
			if (aCafStyle.Material().IsNull()
				&& !theStyle.Material().IsNull())
			{
				aCafStyle.SetMaterial(theStyle.Material());
			}

			TopoDS_Shape aKeyShapeLocated = aKeyShape.Located(theLocation);
			if (aKeyShapeType == TopAbs_FACE)
			{
				myStyles.Bind(aKeyShapeLocated, aCafStyle);
			}
			else
			{
				for (TopExp_Explorer aFaceIter(aKeyShapeLocated, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
				{
					if (!myStyles.IsBound(aFaceIter.Current()))
					{
						myStyles.Bind(aFaceIter.Current(), aCafStyle);
					}
				}
			}
		}
	}
}
