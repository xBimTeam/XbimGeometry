#pragma once
#include <TopoDS_Face.hxx>
#include <BRepLProp_SLProps.hxx>
#include <TopExp_Explorer.hxx>
#include <Poly_Triangulation.hxx>
#include <TDF_Label.hxx>
#include <NCollection_DataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <XCAFPrs_Style.hxx>
#include <BRepTools.hxx>
class NFaceMeshIterator 
{
public:
	NFaceMeshIterator(
		const TDF_Label& theLabel, 
		const XCAFPrs_Style& theStyle,
		const TopLoc_Location& location, 
		double linearDeflection,
		double angularDeflection, 
		bool checkEdges, bool computeNormals);
	NFaceMeshIterator(const TopoDS_Shape& shape, bool checkEdges, bool computeNormals);
	
	//! Return true if iterator points to the valid triangulation.
	bool More() const { return !myPolyTriang.IsNull(); }

	//! Find next value.
	Standard_EXPORT void Next();

	//! Return current face.
	const TopoDS_Face& Face() const { return myFace; }

	bool HasCurves;
	//! Return current face triangulation.
	const Handle(Poly_Triangulation)& Triangulation() const { return myPolyTriang; }

	//! Return true if mesh data is defined.
	bool IsEmptyMesh() const
	{
		return myPolyTriang.IsNull()
			|| (myPolyTriang->NbNodes() < 1 && myPolyTriang->NbTriangles() < 1);
	}
	//! Return true if triangulation has defined normals.
	bool HasNormals() const { return myHasNormals; }

	//! Return true if triangulation has defined normals.
	bool HasTexCoords() const { return !myPolyTriang.IsNull() && myPolyTriang->HasUVNodes(); }
	//! Return normal at specified node index with face transformation applied and face orientation applied.
	gp_Dir NormalTransformed(Standard_Integer theNode)
	{
		gp_Dir aNorm = normal(theNode);
		if (myTrsf.Form() != gp_Identity)
		{
			aNorm.Transform(myTrsf);
		}
		if (myFace.Orientation() == TopAbs_REVERSED)
		{
			aNorm.Reverse();
		}
		return aNorm;
	}

	//! Return number of nodes for the current face.
	Standard_Integer NbNodes() const
	{
		return !myPolyTriang.IsNull()
			? myPolyTriang->NbNodes()
			: 0;
	}

	//! Lower node index in current triangulation.
	Standard_Integer NodeLower() const { return 1; }

	//! Upper node index in current triangulation.
	Standard_Integer NodeUpper() const { return myPolyTriang->NbNodes(); }

	//! Return the node with specified index with applied transformation.
	gp_Pnt NodeTransformed(const Standard_Integer theNode) const
	{
		gp_Pnt aNode = node(theNode);
		aNode.Transform(myTrsf);
		return aNode;
	}

	//! Return texture coordinates for the node.
	gp_Pnt2d NodeTexCoord(const Standard_Integer theNode) const
	{
		return myPolyTriang->HasUVNodes() ? myPolyTriang->UVNode(theNode) : gp_Pnt2d();
	}

	//! Return the node with specified index with applied transformation.
	gp_Pnt node(const Standard_Integer theNode) const { return myPolyTriang->Node(theNode); }

	//! Return normal at specified node index without face transformation applied.
	Standard_EXPORT gp_Dir normal(Standard_Integer theNode);

	

	//! Return triangle with specified index.
	Poly_Triangle triangle(Standard_Integer theElemIndex) const { return myPolyTriang->Triangle(theElemIndex); }

	//! Return number of elements of specific type for the current face.
	Standard_Integer NbTriangles() const { return myPolyTriang->NbTriangles(); }

	//! Lower element index in current triangulation.
	Standard_Integer ElemLower() const { return 1; }

	//! Upper element index in current triangulation.
	Standard_Integer ElemUpper() const { return myPolyTriang->NbTriangles(); }

	//! Return triangle with specified index with applied Face orientation.
	Poly_Triangle TriangleOriented(Standard_Integer theElemIndex) const
	{
		Poly_Triangle aTri = triangle(theElemIndex);
		auto myReversed = (myFace.Orientation() == TopAbs_REVERSED);
		if (myReversed ^ myIsMirrored)
		{
			return Poly_Triangle(aTri.Value(1), aTri.Value(3), aTri.Value(2));
		}
		return aTri;
	}
	//! Return face material.
	const XCAFPrs_Style& FaceStyle() const { return myFaceStyle; }
	//! Reset information for current face.
	void resetFace()
	{
		myPolyTriang.Nullify();
		myFace.Nullify();
		myNodes = NULL;
		myNormals = NULL;		
		myHasNormals = false;
		myHasFaceColor = false;
		myFaceColor = Quantity_ColorRGBA();
		myFaceStyle = XCAFPrs_Style();
	}
private:
	NCollection_DataMap<TopoDS_Shape, XCAFPrs_Style, TopTools_ShapeMapHasher>
		myStyles;       //!< Face -> Style map
	XCAFPrs_Style                   myDefaultStyle;      //!< default material definition to be used for nodes with only color defined
	//! Return face material.
	XCAFPrs_Style					myFaceStyle; 
	bool							myCheckEdges;	//!< if true all face edges are checked to see if they are curved
	TopExp_Explorer                 myFaceIter;     //!< face explorer
	TopoDS_Face                     myFace;         //!< current face
	Handle(Poly_Triangulation)      myPolyTriang;   //!< triangulation of current face
	TopLoc_Location                 myFaceLocation; //!< current face location
	BRepLProp_SLProps               mySLTool;       //!< auxiliary tool for fetching normals from surface
	BRepAdaptor_Surface             myFaceAdaptor;  //!< surface adaptor for fetching normals from surface
	const TColgp_Array1OfPnt*		myNodes;        //!< node positions of current face
	const TShort_Array1OfShortReal* myNormals;      //!< node normals of current face
	bool							myComputeNormals;
	Standard_Boolean                myHasNormals;   //!< flag indicating that current face has normals
	gp_Trsf                         myTrsf;         //!< current face transformation
	Standard_Boolean                myIsMirrored;   //!< flag indicating that face triangles should be mirrored
	Quantity_ColorRGBA              myFaceColor;    //!< current face color
	Standard_Boolean                myHasFaceColor; //!< flag indicating that current face has assigned color
	double							myAngularDeflection;
	double							myLinearDeflection;
	TopoDS_Shape					myShape;
	
	//! Initialize face properties.
	void initFace();
	bool hasCurves();
	void dispatchStyles(const TDF_Label& theLabel, const TopLoc_Location& theLocation, const XCAFPrs_Style& theStyle);
};

