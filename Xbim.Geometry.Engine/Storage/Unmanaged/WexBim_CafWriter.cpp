// Copyright (c) 2017-2019 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include "WexBim_CafWriter.h"

#include <gp_Quaternion.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_ProgressScope.hxx>
#include <NCollection_DataMap.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <Poly_Triangulation.hxx>
#include <RWGltf_GltfAccessorLayout.hxx>
#include <RWGltf_GltfMaterialMap.hxx>
#include <RWGltf_GltfPrimitiveMode.hxx>
#include <RWGltf_GltfRootElement.hxx>
#include <RWGltf_GltfSceneNodeMap.hxx>
#include <RWMesh_FaceIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_DocumentExplorer.hxx>
#include <BRep_Tool.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <Geom_Plane.hxx>
#include <NCollection_Vec3.hxx>
#include <CDM_MetaData.hxx>
#include <CDM_MetaDataLookUpTable.hxx>
#include <TNaming_UsedShapes.hxx>
#include <TNaming_PtrRefShape.hxx>
#include <TNaming_DataMapOfShapePtrRefShape.hxx>
#include <TNaming_DataMapIteratorOfDataMapOfShapePtrRefShape.hxx>
#include <TNaming_RefShape.hxx>

#include "FlexShape.h"
#include "ShapeNode.h"
#include "Graphic3d_BndBox3f.h"
#include "NFaceMeshIterator.h"
#include <TDF_ChildIterator.hxx>

#include "WexBimStyles.h"
#include "WexBimShape.h"
#include "WexBimGeometryModel.h"
#include "WexBimRegion.h"
#include "WexBimStreamContent.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <unordered_map>
#include <map>
#include <XCAFDoc_Location.hxx>

IMPLEMENT_STANDARD_RTTIEXT(WexBim_CafWriter, Standard_Transient)



namespace
{
	//! Write three float values.
	static void writeVec3(std::ostream& theStream,
		const gp_XYZ& theVec3)
	{
		Graphic3d_Vec3 aVec3(float(theVec3.X()), float(theVec3.Y()), float(theVec3.Z()));
		theStream.write((const char*)aVec3.GetData(), sizeof(aVec3));
	}

	//! Write three float values.
	static void writeVec3(std::ostream& theStream,
		const Graphic3d_Vec3& theVec3)
	{
		theStream.write((const char*)theVec3.GetData(), sizeof(theVec3));
	}

	//! Write two float values.
	static void writeVec2(std::ostream& theStream,
		const gp_XY& theVec2)
	{
		Graphic3d_Vec2 aVec2(float(theVec2.X()), float(theVec2.Y()));
		theStream.write((const char*)aVec2.GetData(), sizeof(aVec2));
	}

	//! Write triangle indices.
	static void writeTriangle32(std::ostream& theStream,
		const Graphic3d_Vec3i& theTri)
	{
		theStream.write((const char*)theTri.GetData(), sizeof(theTri));
	}

	//! Write triangle indices.
	static void writeTriangle16(std::ostream& theStream,
		const NCollection_Vec3<uint16_t>& theTri)
	{
		theStream.write((const char*)theTri.GetData(), sizeof(theTri));
	}

}

//================================================================
// Function : Constructor
// Purpose  :
//================================================================
WexBim_CafWriter::WexBim_CafWriter(
	const Quantity_ColorRGBA& defaultColour,
	double oneMeter,
	double linearDeflection,
	double angularDeflection,
	double precision) :
	_oneMeter(oneMeter),
	_linearDeflection(linearDeflection),
	_angularDeflection(angularDeflection),
	_precision(precision)
{
	myCSTrsf.SetInputLengthUnit(1);
	myCSTrsf.SetOutputLengthUnit(oneMeter); // meters will be the units for wexbim
	myCSTrsf.SetOutputCoordinateSystem(RWMesh_CoordinateSystem_Zup); //wexbim has Z not Y as up
	//set default undefined to a mid grey
	myDefaultStyle.SetColorSurf(Quantity_NOC_GRAY40);
	myDefaultStyle.SetMaterial(new XCAFDoc_VisMaterial());
	XCAFDoc_VisMaterialCommon aMatCom = myDefaultStyle.Material()->CommonMaterial();
	aMatCom.DiffuseColor = defaultColour.GetRGB();
	aMatCom.AmbientColor = defaultColour.GetRGB();
	aMatCom.Transparency = 1.0f - defaultColour.Alpha();
	aMatCom.IsDefined = true;
	myDefaultStyle.Material()->SetCommonMaterial(aMatCom);
}


//================================================================
// Function : Destructor
// Purpose  :
//================================================================
WexBim_CafWriter::~WexBim_CafWriter()
{

}


// =======================================================================
// function : saveNodes
// purpose  :
// =======================================================================
void WexBim_CafWriter::saveNodes(NWexBimMesh& theMesh, const NFaceMeshIterator& theFaceIter, std::vector<int>& pointIndexMap) const
{
	const Standard_Integer aNodeUpper = theFaceIter.NodeUpper();
	for (Standard_Integer aNodeIter = theFaceIter.NodeLower(); aNodeIter <= aNodeUpper; ++aNodeIter)
	{
		gp_XYZ aNode = theFaceIter.NodeTransformed(aNodeIter).XYZ();
		myCSTrsf.TransformPosition(aNode);
		theMesh.BndBox.Add(BVH_Vec3f((float)(aNode.X()), (float)(aNode.Y()), (float)(aNode.Z())));
		pointIndexMap.push_back(theMesh.AddPoint(aNode));
	}
}

// =======================================================================
// function : saveIndices
// purpose  :
// =======================================================================
void WexBim_CafWriter::saveIndicesAndNormals(NWexBimMesh& wexBimMesh, NFaceMeshIterator& theFaceIter, std::vector<int>& pointIndexMap)
{
	const Standard_Integer anElemLower = theFaceIter.ElemLower();
	const Standard_Integer anElemUpper = theFaceIter.ElemUpper();
	TriangleIndices triangleIndices;
	Handle(Geom_Surface) surf = BRep_Tool::Surface(theFaceIter.Face()); //the surface
	Handle(Geom_Plane) hPlane = Handle(Geom_Plane)::DownCast(surf);
	bool isPlanar = !hPlane.IsNull();
	VectorOfPackedNormal normals;
	bool hasNormals = theFaceIter.HasNormals();
	int numNormals = 3; //one for each triangle indices

	for (Standard_Integer anElemIter = anElemLower; anElemIter <= anElemUpper; ++anElemIter)
	{
		Poly_Triangle aTri = theFaceIter.TriangleOriented(anElemIter);

		triangleIndices.Append(Vec3OfInt(pointIndexMap[aTri(1) - 1], pointIndexMap[aTri(2) - 1], pointIndexMap[aTri(3) - 1]));
		if (isPlanar && hasNormals) //jsut need one and then stop
		{
			const gp_Dir aNormal = theFaceIter.NormalTransformed(aTri(1));
			Graphic3d_Vec3 aVecNormal((float)aNormal.X(), (float)aNormal.Y(), (float)aNormal.Z());
			aVecNormal.Normalize();
			myCSTrsf.TransformNormal(aVecNormal);
			normals.push_back(NWexBimMesh::ToPackedNormal(aVecNormal));
			hasNormals = false; //stop further processing
		}
		else if (hasNormals)
		{
			for (int i = 1; i <= numNormals; i++)
			{
				const gp_Dir aNormal = theFaceIter.NormalTransformed(aTri(i));
				Graphic3d_Vec3 aVecNormal((float)aNormal.X(), (float)aNormal.Y(), (float)aNormal.Z());
				aVecNormal.Normalize();
				myCSTrsf.TransformNormal(aVecNormal);
				normals.push_back(NWexBimMesh::ToPackedNormal(aVecNormal));
			}
		}

	}
	wexBimMesh.AddTriangleIndices(triangleIndices);
	wexBimMesh.AddNormals(normals);
}

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
bool WexBim_CafWriter::Perform(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
	const TColStd_IndexedDataMapOfStringString& theFileInfo)
{
	//get all the Ifc Products from the root level
	TDF_LabelSequence aRoots;
	Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDocument->Main());
	aShapeTool->GetShapes(aRoots);
	TDF_LabelSequence products;
	for (TDF_LabelSequence::Iterator aShapeRepIt(aRoots); aShapeRepIt.More(); aShapeRepIt.Next())
	{
		const TDF_Label& aShapeLab = aShapeRepIt.Value();
		if (FlexShape::IsProduct(aShapeLab))
			products.Append(aShapeLab);
	}
	return Perform(theDocument, outputStrm, products, theFileInfo);
}



// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
bool WexBim_CafWriter::Perform(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
	const TDF_LabelSequence& theRootLabels,
	const TColStd_IndexedDataMapOfStringString& theFileInfo)
{
	return writeBinData(theDocument, outputStrm, theRootLabels, theFileInfo);
}

// =======================================================================
// function : writeBinData
// purpose  : writes out the WexBimMesh content
// =======================================================================
bool WexBim_CafWriter::writeBinData(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
	const TDF_LabelSequence& theRootLabels,
	const TColStd_IndexedDataMapOfStringString& theFileInfo)
{


	WexBimStreamContent streamContent(myDefaultStyle);
	WexBimRegion& wexBimRegion = streamContent.AddRegion();

	int productId = -1;
	short productTypeId = -1;
	int shapeId = -1;
	ShapeNode node;

	std::unordered_map<ShapeNode, std::vector<WexBimShape>, ShapeNode::ShapeNodeHashFunction> shapes;

	for (XCAFPrs_DocumentExplorer aDocExplorer(theDocument, theRootLabels, XCAFPrs_DocumentExplorerFlags_None);
		aDocExplorer.More(); aDocExplorer.Next())
	{
		const XCAFPrs_DocumentNode& aDocNode = aDocExplorer.Current();
		if (FlexShape::IsProduct(aDocNode.Label))
		{
			//if a product and a shape have already been found and we are starting a new one
			if (shapeId != -1 && productId != -1 && productTypeId != -1)
			{
				WexBimProduct& product = streamContent.AddProduct(productId, productTypeId);
				WexBimShape shape(productId, shapeId, streamContent.Styles.AddMaterial(node.Style));
				auto& record = shapes.try_emplace(node, std::vector<WexBimShape>()); //make sure we have a collection in place
				record.first->second.push_back(shape); //add the shape in
				shapeId = -1;
			}
			productId = FlexShape::IfcId(aDocNode.Label);
			productTypeId = FlexShape::IfcTypeId(aDocNode.Label);
			node = aDocNode;
			shapeId = -1;
			continue;
		}
		else
		{
			node = aDocNode;
			if (FlexShape::IsShapeRepresentation(aDocNode.Label))
			{
				shapeId = FlexShape::IfcId(aDocNode.Label);
			}
		}

	}
	//add in the last one if we have one
	if (shapeId != -1 && productId != -1 && productTypeId != -1)
	{
		WexBimProduct& product = streamContent.AddProduct(productId, productTypeId);
		WexBimShape shape(productId, shapeId, streamContent.Styles.AddMaterial(node.Style));
		auto& record = shapes.try_emplace(node, std::vector<WexBimShape>()); //make sure we have a collection in place
		record.first->second.push_back(shape); //add the shape in
	}
	//convert the meshes
	//just iterate over unique keys

	for (auto& shapeRow : shapes)
	{

		TDF_LabelSequence nodes;
		ShapeNode node = shapeRow.first;

		nodes.Append(node.ShapeLabel);
		std::unordered_map<XCAFPrs_Style, int, StyleHasher> geometryModelStyleMap;
		std::vector<WexBimGeometryModel> geomModels;
		//find the actual wexBimShape List;
		auto& shapesIt = shapes.find(node);
		const std::vector<WexBimShape>& wexBimShapes = shapesIt->second;
		for (TDF_ChildIterator childIt(node.ShapeLabel, true); childIt.More(); childIt.Next())
		{
			TopLoc_Location shapeLoc = XCAFDoc_ShapeTool::GetLocation(childIt.Value());
			shapeLoc = node.Location.Multiplied(shapeLoc);
			for (NFaceMeshIterator aFaceIter(
				childIt.Value(),
				node.Style,
				wexBimShapes.size() == 1 ? shapeLoc : TopLoc_Location(), //apply transform to the mesh if only one instance
				_linearDeflection,
				_angularDeflection,
				false);
				aFaceIter.More();
				aFaceIter.Next())
			{
				if (aFaceIter.IsEmptyMesh())
					continue;

				auto& geomModelPair = geometryModelStyleMap.find(aFaceIter.FaceStyle());
				int styleId = streamContent.Styles.AddMaterial(aFaceIter.FaceStyle());
				if (geomModelPair != geometryModelStyleMap.cend()) //we have done this style on this shape already
				{
					WexBimGeometryModel& geomModel = geomModels[geomModelPair->second];
					std::vector<int> pointIndexMap;
					saveNodes(geomModel.Mesh(), aFaceIter, pointIndexMap);
					saveIndicesAndNormals(geomModel.Mesh(), aFaceIter, pointIndexMap);

				}
				else //create a new geometry model for a sub shape/colour
				{
					WexBimGeometryModel geomModel(_precision);
					gp_Trsf transform = shapeLoc.Transformation();
					myCSTrsf.TransformTransformation(transform); //adjust to wcs
					for (auto& wexBimShape : wexBimShapes)
					{
						WexBimShape copyShape(wexBimShape.ProductId,wexBimShape.InstanceId, styleId, transform);					
						geomModel.Shapes.push_back(copyShape);
					}

					std::vector<int> pointIndexMap;

					saveNodes(geomModel.Mesh(), aFaceIter, pointIndexMap);
					saveIndicesAndNormals(geomModel.Mesh(), aFaceIter, pointIndexMap);

					geometryModelStyleMap.insert(std::make_pair(aFaceIter.FaceStyle(), (int) geomModels.size()));
					geomModels.push_back(geomModel);
				}
			} //the mesh is complete for all shapes		
		}
		for (auto& geomModel : geomModels)
		{
			wexBimRegion.GeometryModels.push_back(geomModel);
			//update the product bounding boxes
			for (auto& shape : geomModel.Shapes)
			{
				auto& productRec = streamContent.Products.find(shape.ProductId);
				productRec->second.BndBox.Combine(geomModel.Mesh().BndBox);
			}
		}
	}

	//calculate the entire bounds of the region
	for (auto& it = wexBimRegion.GeometryModels.begin(); it != wexBimRegion.GeometryModels.end(); it++)
	{
		wexBimRegion.BndBox.Combine(it->Mesh().BndBox);
	}

	//commit  to wexbim stream
	streamContent.WriteWexBimStream(outputStrm);
	return true;
}




