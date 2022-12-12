#include "StorageItem.h"
#include "../BRep/XShape.h"
#include "../Visual/VisualMaterial.h"
#include "../Exceptions/XbimGeometryServiceException.h"
#include "../BRep/XCompound.h"
#include "../BRep/XSolid.h"
#include "../BRep/XShell.h"
#include "../BRep/XFace.h"
#include "../BRep/XWire.h"
#include <XCAFDoc_DocumentTool.hxx>
#include <TDataStd_Name.hxx>
#include <BRep_Builder.hxx>

#include "../Factories//ShapeFactory.h"
#include <TopoDS.hxx>
#include <XCAFDoc_Volume.hxx>
#include <XCAFDoc_Location.hxx>

#include <TDataStd_Integer.hxx>
#include <TDataStd_BooleanArray.hxx>
#include <TDataStd_NamedData.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <TDataStd_TreeNode.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>
#include "Unmanaged/FlexShape.h"
#include "../XbimConvert.h"

using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Factories;

using namespace Xbim::Geometry::Storage;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			

			void StorageItem::AddReference(IXStorageItem^ referred, System::Nullable<XbimMatrix3D> transform)
			{

				const TDF_Label& referedLabel = static_cast<StorageItem^>(referred)->Ref();
				if (transform.HasValue && !transform.Value.IsIdentity) //adjust the transformation
				{
					gp_Trsf occTrans = XbimConvert::ToTransform(transform.Value);
					TopLoc_Location occLoc(occTrans);
					TDF_Label aLabel = The_ShapeTool()->AddComponent(Ref(), referedLabel, occLoc);
					SetName(aLabel, "R");
				}
				else
				{
					TDF_Label aLabel = The_ShapeTool()->AddComponent(Ref(), referedLabel, TopLoc_Location());
					SetName(aLabel, "R");					
					
				}
			}




			IXStorageItem^ StorageItem::AddShape(System::String^ name, IXShape^ shape, bool expand)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				TDF_Label aLabel = The_ShapeTool()->AddShape(topoShape, expand);
				SetName(aLabel, name);
				return gcnew StorageItem(aLabel);
			}

			void StorageItem::SetPlacement(IIfcObjectPlacement^ placement)
			{
				TopLoc_Location location = XbimConvert::ToLocation(placement, nullptr);
				XCAFDoc_Location::Set(Ref(), location);
			}

			IXStorageItem^ StorageItem::AddAssembly(System::String^ subAssemblyName)
			{
				return AddAssembly(subAssemblyName, nullptr);
			}



			IXStorageItem^ StorageItem::AddAssembly(System::String^ subAssemblyName, IIfcObjectPlacement^ placement)
			{
				if (!(this->IsAssembly || this->IsSimpleShape))
					throw gcnew XbimGeometryException("Assemblies can only be added to Assemblies or simple shapes");
				TDF_Label subAssembly = The_ShapeTool()->NewShape();
				TopLoc_Location location;
				if (placement != nullptr)
					location = XbimConvert::ToLocation(placement, nullptr);
				TDF_Label aLabel = The_ShapeTool()->AddComponent(Ref(), subAssembly, location);
				SetName(aLabel, subAssemblyName);
				SetName(subAssembly, subAssemblyName);
				return gcnew StorageItem(subAssembly);
			}

			IXStorageItem^ StorageItem::AddAssembly(System::String^ subAssemblyName, XbimMatrix3D transform)
			{
				if (!(this->IsAssembly || this->IsSimpleShape)) throw gcnew XbimGeometryException("Assemblies can only be added to Assemblies or simple shapes");
				TDF_Label subAssemblyLabel = The_ShapeTool()->NewShape();
				gp_Trsf occTrans = XbimConvert::ToTransform(transform);

				TDF_Label aLabel;
				if (!transform.IsIdentity) //adjust the transformation
				{
					TopLoc_Location occLoc(occTrans);
					aLabel = The_ShapeTool()->AddComponent(Ref(), subAssemblyLabel, occLoc);
					SetName(aLabel, subAssemblyName);
				}
				else
				{
					aLabel = The_ShapeTool()->AddComponent(Ref(), subAssemblyLabel, TopLoc_Location());
					SetName(aLabel, subAssemblyName);
				}
				SetName(subAssemblyLabel, subAssemblyName);
				return gcnew StorageItem(subAssemblyLabel);
			}

			IXStorageItem^ StorageItem::AddComponent(System::String^ name, IXStorageItem^ component, IIfcObjectPlacement^ objPlacement, ILogger^ logger)
			{

				TDF_Label label = static_cast<StorageItem^>(component)->Ref();
				//add the shape to the parent to avoid having to call update assemblies
				TDF_Label componentLabel = The_ShapeTool()->AddComponent(Ref(), label, XbimConvert::ToLocation(objPlacement, logger));
				SetName(componentLabel, name);
				StorageItem^ shapeRepNode = gcnew StorageItem(componentLabel);
				shapeRepNode->IsShapeRepresentation = true;
				return shapeRepNode;

			}

			IXStorageItem^ StorageItem::AddComponent(System::String^ name, int shapeId, IXShape^ shape)
			{
				return AddComponent(name, shapeId, shape, nullptr, nullptr);
			}


			IXStorageItem^ StorageItem::AddComponent(System::String^ name, IXStorageItem^ componentItem, XbimMatrix3D transform)
			{
				if (!(this->IsAssembly || this->IsSimpleShape)) throw gcnew XbimGeometryException("Components can only be added to Assemblies or simple shapes");
				//add the shape to the parent to avoid having to call update assemblies
				TopoDS_Shape mainShape = The_ShapeTool()->GetShape(Ref());
				gp_Trsf occTrans = XbimConvert::ToTransform(transform);
				TDF_Label storageLabel = static_cast<StorageItem^>(componentItem)->Ref();
				TopLoc_Location occLoc(occTrans);
				TDF_Label	aLabel = The_ShapeTool()->AddComponent(Ref(), storageLabel, occLoc);
				SetName(aLabel, name);
				return gcnew StorageItem(aLabel);
			}

			IXStorageItem^ StorageItem::AddComponent(System::String^ name, int shapeId, IXShape^ shape, IIfcObjectPlacement^ objPlacement, ILogger^ logger)
			{
				if (!(this->IsAssembly || this->IsSimpleShape)) throw gcnew XbimGeometryException("Components can only be added to Assemblies or simple shapes");

				const TopoDS_Shape& topoShape = ((XShape^)shape)->GetTopoShape();

				TopoDS_Shape shapeWithoutLocation = topoShape;
				TopLoc_Location emptyLocation;
				shapeWithoutLocation.Location(emptyLocation);
				TopLoc_Location l = shapeWithoutLocation.Location();
				TDF_Label componentLabel = The_ShapeTool()->FindShape(shapeWithoutLocation);
				if (componentLabel.IsNull()) //this basic shape has not been addded before
				{
					//add it to the shape list in the document
					componentLabel = The_ShapeTool()->AddShape(shapeWithoutLocation);
					System::String^ shapeIdStr = System::Convert::ToString(shapeId);
					SetName(componentLabel, shapeIdStr);
				}
				TopLoc_Location location = topoShape.Location();
				if (objPlacement != nullptr)
				{
					location = location.Multiplied(XbimConvert::ToLocation(objPlacement, logger));
				}
				componentLabel = The_ShapeTool()->AddComponent(Ref(), componentLabel, location);
				SetName(componentLabel, name);
				return gcnew StorageItem(componentLabel);


			}

			IXStorageItem^ StorageItem::AddSubShape(System::String^ name, IXShape^ shape, bool isExistingPart)
			{
				//first ensure the shape of the label has this shape in its compounds
				const TopoDS_Shape& subShape = ((XShape^)shape)->GetTopoShape();

				if (!isExistingPart)
				{
					TopoDS_Shape mainShape = The_ShapeTool()->GetShape(Ref());
					BRep_Builder b;
					b.Add(mainShape, subShape);
					The_ShapeTool()->SetShape(Ref(), mainShape);
				}
				TDF_Label addedSubShape;
				bool ok = The_ShapeTool()->AddSubShape(Ref(), subShape, addedSubShape);

				if (ok)
				{
					SetName(addedSubShape, name);
					return gcnew StorageItem(addedSubShape);
				}
				else
					throw gcnew System::Exception("Shape is not a sub shape of the selected assembly");

			}

			int StorageItem::IfcId::get()
			{
				return FlexShape::IfcId(Ref());
			}

			void StorageItem::IfcId::set(int id)
			{
				SetIntAttribute("IfcId", id);
			}
			short StorageItem::IfcTypeId::get()
			{
				return FlexShape::IfcTypeId(Ref());
			}

			void StorageItem::IfcTypeId::set(short id)
			{
				SetIntAttribute("IfcTypeId", id);
			}
			int StorageItem::NbMaterialLayers::get()
			{
				return GetIntAttribute("NbMaterialLayers");
			}

			void StorageItem::NbMaterialLayers::set(int nbMaterialLayers)
			{
				SetIntAttribute("NbMaterialLayers", nbMaterialLayers);
			}


			System::String^ StorageItem::IfcName::get()
			{
				return GetStringAttribute("IfcName");
			}

			void StorageItem::IfcName::set(System::String^ name)
			{
				SetStringAttribute("IfcName", name);
			}
			System::String^ StorageItem::IfcType::get()
			{
				return GetStringAttribute("IfcType");
			}

			void StorageItem::IfcType::set(System::String^ name)
			{
				SetStringAttribute("IfcType", name);
			}

			System::String^ StorageItem::Classification::get()
			{
				return GetStringAttribute("Classification");
			}

			void StorageItem::Classification::set(System::String^ classification)
			{
				SetStringAttribute("Classification", classification);
			}

			bool StorageItem::HasOpenings::get()
			{
				Handle(TDataStd_BooleanArray)features;
				bool hasFeatures = Ref().FindAttribute(TDataStd_BooleanArray::GetID(), features);
				if (hasFeatures) return features->Value(1); else return false;
			}
			void StorageItem::HasOpenings::set(bool hasOpenings)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(1, hasOpenings);
			}
			bool StorageItem::IsProduct::get()
			{
				return FlexShape::IsProduct(Ref());
			}
			void StorageItem::IsProduct::set(bool isProduct)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(3, isProduct);
			}

			bool StorageItem::IsShapeRepresentation::get()
			{
				return FlexShape::IsShapeRepresentation(Ref());
			}

			void StorageItem::IsShapeRepresentation::set(bool isShapeRepresentation)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(4, isShapeRepresentation);
			}

			bool StorageItem::IsShapeRepresentationMap::get()
			{
				return FlexShape::IsShapeRepresentationMap(Ref());
			}

			void StorageItem::IsShapeRepresentationMap::set(bool isShapeRepresentationMap)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(6, isShapeRepresentationMap);
			}

			bool StorageItem::IsGeometricRepresentationItem::get()
			{
				return FlexShape::IsGeometricRepresentationItem(Ref());
			}
			void StorageItem::IsGeometricRepresentationItem::set(bool isShapeRepresentation)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(5, isShapeRepresentation);
			}
			IXVisualMaterial^ StorageItem::VisualMaterial::get()
			{
				TDF_Label  materialLabel;
				try
				{
					Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref());

					if (aMatTool->GetShapeMaterial(Ref(), materialLabel))
					{
						Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(materialLabel);
						return gcnew Xbim::Geometry::Visual::VisualMaterial(aMat, gcnew StorageItem(materialLabel));
					}
					return nullptr;
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					throw gcnew XbimGeometryServiceException(gcnew System::String(strm.str().c_str()));
				}
				
			}

			void StorageItem::VisualMaterial::set(IXVisualMaterial^ material)
			{
				if (!material->IsStored) return;
				StorageItem^ materialItem = dynamic_cast<StorageItem^>(material->Label);
				Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(materialItem->Ref());
				if (materialItem == nullptr || !aMatTool->IsMaterial(materialItem->Ref()))
					throw gcnew XbimGeometryServiceException("Visual Material not found:" + material->Name);
				aMatTool->SetShapeMaterial(Ref(), materialItem->Ref());
			}

			bool StorageItem::HasProjections::get()
			{
				Handle(TDataStd_BooleanArray)features;
				bool hasFeatures = Ref().FindAttribute(TDataStd_BooleanArray::GetID(), features);
				if (hasFeatures) return features->Value(2); else return false;
			}
			void StorageItem::HasProjections::set(bool hasProjections)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(2, hasProjections);
			}
			System::String^ StorageItem::GetStringAttribute(System::String^ attributeName)
			{
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					TCollection_AsciiString val;
					if (FlexShape::GetStringAttribute(Ref(), pAnsiName, val))
						return gcnew System::String(val.ToCString());
					else
						return nullptr;
				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);
				}

			}

			void StorageItem::SetStringAttribute(System::String^ attributeName, System::String^ attributeValue)
			{
				if (System::String::IsNullOrWhiteSpace(attributeName) || System::String::IsNullOrWhiteSpace(attributeValue)) return;
				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);
				System::IntPtr pAttributeValue = Marshal::StringToHGlobalAnsi(attributeValue);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					const char* pAnsiValue = static_cast<const char*>(pAttributeValue.ToPointer());
					namedData->SetString(pAnsiName, pAnsiValue);
				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);
					Marshal::FreeHGlobal(pAttributeValue);
				}

			}

			int StorageItem::GetIntAttribute(System::String^ attributeName)
			{
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					return FlexShape::GetIntAttribute(Ref(), pAnsiName);
				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);
				}

			}

			void StorageItem::SetIntAttribute(System::String^ attributeName, int attributeValue)
			{
				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);

				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					namedData->SetInteger(pAnsiName, attributeValue);

				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);

				}
			}

			System::Nullable<double> StorageItem::GetDoubleAttribute(System::String^ attributeName)
			{
				Handle(TDataStd_NamedData) attributes;
				bool hasAttributes = Ref().FindAttribute(TDataStd_NamedData::GetID(), attributes);
				if (!hasAttributes) return 0;
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					if (attributes->HasReal(pAnsiName))
						return attributes->GetReal(pAnsiName);
					else
						return System::Nullable<double>();
				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);
				}
			}

			int StorageItem::NbComponents::get()
			{
				return The_ShapeTool()->NbComponents(Ref());
			}

			IEnumerable<IXStorageItem^>^ StorageItem::Components::get()
			{
				TDF_LabelSequence labels;
				bool found = The_ShapeTool()->GetComponents(Ref(), labels);
				if (!found) return Enumerable::Empty<IXStorageItem^>();
				List<IXStorageItem^>^ components = gcnew List<IXStorageItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					components->Add(gcnew StorageItem(*it));
				return components;
			}

			IEnumerable<IXShape^>^ StorageItem::SubShapes::get()
			{
				TDF_LabelSequence labels;
				bool found = The_ShapeTool()->GetSubShapes(Ref(), labels);
				if (!found) return Enumerable::Empty<IXShape^>();
				List<IXShape^>^ components = gcnew List<IXShape^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
				{
					TopoDS_Shape topoShape = The_ShapeTool()->GetShape(*it);
					components->Add(ShapeFactory::GetXbimShape(topoShape));
				}
				return components;
			}

			IEnumerable<IXStorageItem^>^ StorageItem::SubShapeStorageItems::get()
			{
				TDF_LabelSequence labels;
				bool found = The_ShapeTool()->GetSubShapes(Ref(), labels);
				if (!found) return Enumerable::Empty<IXStorageItem^>();
				List<IXStorageItem^>^ components = gcnew List<IXStorageItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					components->Add(gcnew StorageItem(*it));
				return components;
			}

			IXStorageItem^ StorageItem::ReferredShape::get()
			{
				TDF_Label refLabel;
				bool found = The_ShapeTool()->GetReferredShape(Ref(), refLabel);
				if (!found || refLabel.IsNull()) return nullptr;
				else
					return gcnew StorageItem(refLabel);
			}
			void StorageItem::SetDoubleAttribute(System::String^ attributeName, System::Nullable<double> attributeValue)
			{

				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = Marshal::StringToHGlobalAnsi(attributeName);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					if (attributeValue.HasValue)
					{
						namedData->SetReal(pAnsiName, attributeValue.Value);
					}
					else //forget it
					{
						namedData->ForgetAttribute(namedData->GetID());
					}

				}
				finally
				{
					Marshal::FreeHGlobal(pAttributeName);

				}

			}
			TopoDS_Shape StorageItem::GetShape()
			{
				return The_ShapeTool()->GetShape(Ref());
			}
			void StorageItem::SetShape(const TopoDS_Shape shape)
			{
				return The_ShapeTool()->SetShape(Ref(), shape);
			}

			////void StorageItem::SetPhysicalMaterial(String^ materialName)
			////{
			////	IntPtr pShape = Marshal::StringToHGlobalAnsi(materialName);
			////	try
			////	{					
			////		Handle(XCAFDoc_MaterialTool) materialTool = XCAFDoc_DocumentTool::MaterialTool(Ref());
			////		const char* pMaterialName = static_cast<const char*>(pShape.ToPointer());
			////		Handle(TCollection_HAsciiString) emptyString = new TCollection_HAsciiString("");//we can add description and density data here if required
			////		Handle(TCollection_HAsciiString) materialName = new TCollection_HAsciiString(pMaterialName);
			////		double density = -1;
			////		materialTool->SetMaterial(Ref(), materialName, emptyString, density, emptyString, emptyString);					
			////	}
			////	finally
			////	{
			////		Marshal::FreeHGlobal(pShape);
			////	}
			////}

			IXShape^ StorageItem::Shape::get()
			{
				TopoDS_Shape topoShape = The_ShapeTool()->GetShape(Ref());
				return ShapeFactory::GetXbimShape(topoShape);
			}
			void StorageItem::Shape::set(IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				The_ShapeTool()->SetShape(Ref(), topoShape);

			}


			System::Nullable<double> StorageItem::Volume::get()
			{

				return GetDoubleAttribute("Volume");
			}
			void StorageItem::Volume::set(System::Nullable<double> volume)
			{
				SetDoubleAttribute("Volume", volume);
			}
			System::Nullable<double> StorageItem::Area::get()
			{
				return GetDoubleAttribute("Area");
			}
			void StorageItem::Area::set(System::Nullable<double> area)
			{
				SetDoubleAttribute("Area", area);
			}
			System::Nullable<double> StorageItem::HeightMax::get()
			{
				return GetDoubleAttribute("HeightMax");
			}
			void StorageItem::HeightMax::set(System::Nullable<double> heightMax)
			{
				SetDoubleAttribute("HeightMax", heightMax);
			}
			System::Nullable<double> StorageItem::HeightMin::get()
			{
				return GetDoubleAttribute("HeightMin");
			}
			void StorageItem::HeightMin::set(System::Nullable<double> heightMin)
			{
				SetDoubleAttribute("HeightMin", heightMin);
			}

			System::Nullable<double> StorageItem::ThicknessMax::get()
			{
				return GetDoubleAttribute("ThicknessMax");
			}
			void StorageItem::ThicknessMax::set(System::Nullable<double> thickness)
			{
				SetDoubleAttribute("ThicknessMax", thickness);
			}

			System::String^ StorageItem::Key::get()
			{
				TCollection_AsciiString entry;
				TDF_Tool::Entry(Ref(), entry);
				return gcnew System::String(entry.ToCString());
				
			}

			System::String^ StorageItem::Name::get()
			{
				Handle(TDataStd_Name) name;
				bool hasName = Ref().FindAttribute(TDataStd_Name::GetID(), name);
				if (hasName)
					return gcnew System::String(name->Get().ToWideString());
				return
					System::String::Empty;
			}

			void StorageItem::Name::set(System::String^ name)
			{
				SetName(Ref(), name);
			}
			void StorageItem::SetName(const TDF_Label& label, System::String^ name)
			{
				if (!label.IsNull() && name != nullptr)
				{
					System::IntPtr p = Marshal::StringToHGlobalAnsi(name);
					try
					{
						const char* pAnsi = static_cast<const char*>(p.ToPointer());
						TDataStd_Name::Set(label, pAnsi);
					}
					finally
					{
						Marshal::FreeHGlobal(p);
					}
				}
			}

			IEnumerable<IXStorageItem^>^ StorageItem::ShapesAssignedToMaterial::get()
			{
				TDF_LabelSequence userLabels;
				int numUsers = The_ShapeTool()->GetUsers(Ref(), userLabels, true);
				List<IXStorageItem^>^ users = gcnew List<IXStorageItem^>(numUsers);
				Handle(TDataStd_TreeNode) tree;
				bool hasTree = Ref().FindAttribute(XCAFDoc::VisMaterialRefGUID(), tree);
				if (hasTree)
				{
					int nbKids = tree->NbChildren(true);
					Handle(TDataStd_TreeNode) f = tree->First();
					for (int i = 0; i < nbKids; i++)
					{
						users->Add(gcnew StorageItem(f->Label()));
						f = f->Next();
					}
				}
				return users;
			}
		}
	}
}