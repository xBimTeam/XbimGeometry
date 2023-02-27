#include "BRepDocumentItem.h"
#include "../BRep/XShape.h"
#include "../Visual/VisualMaterial.h"

#include "../BRep/XCompound.h"
#include "../BRep/XSolid.h"
#include "../BRep/XShell.h"
#include "../BRep/XFace.h"
#include "../BRep/XWire.h"

#include <TDataStd_Name.hxx>
#include <BRep_Builder.hxx>

#include "../Factories//ShapeFactory.h"
#include <TopoDS.hxx>
//#include <XCAFDoc_Volume.hxx>
#include <XCAFDoc_Location.hxx>

#include <TDataStd_Integer.hxx>
#include <TDataStd_BooleanArray.hxx>
#include <TDataStd_NamedData.hxx>
//#include <XCAFDoc_MaterialTool.hxx>
#include <TDataStd_TreeNode.hxx>
//#include <XCAFDoc.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>

#include "../XbimConvert.h"
#include "unmanaged/FlexShape.h"
#include <XCAFDoc.hxx>
#include "unmanaged/NBRepDocument.h"

using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Factories;
using namespace Xbim::Geometry::Visual;
using namespace Xbim::Geometry::Storage;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{


			void BRepDocumentItem::AddReference(IXBRepDocumentItem^ referred, System::Nullable<XbimMatrix3D> transform)
			{

				const TDF_Label& referedLabel = static_cast<BRepDocumentItem^>(referred)->Ref();
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




			IXBRepDocumentItem^ BRepDocumentItem::AddShape(System::String^ name, IXShape^ shape, bool expand)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				TDF_Label aLabel = The_ShapeTool()->AddShape(topoShape, expand);
				SetName(aLabel, name);
				return gcnew BRepDocumentItem(aLabel, _modelServices);
			}

			void BRepDocumentItem::SetPlacement(IIfcObjectPlacement^ placement)
			{
				TopLoc_Location location = XbimConvert::ToLocation(placement, _modelServices->Logger(), _modelServices);
				XCAFDoc_Location::Set(Ref(), location);
			}

			IXBRepDocumentItem^ BRepDocumentItem::AddAssembly(System::String^ subAssemblyName)
			{
				return AddAssembly(subAssemblyName, nullptr);
			}



			IXBRepDocumentItem^ BRepDocumentItem::AddAssembly(System::String^ subAssemblyName, IIfcObjectPlacement^ placement)
			{
				if (!(this->IsAssembly || this->IsSimpleShape))
					throw gcnew XbimGeometryException("Assemblies can only be added to Assemblies or simple shapes");
				TDF_Label subAssembly = The_ShapeTool()->NewShape();
				TopLoc_Location location;
				if (placement != nullptr)
					location = XbimConvert::ToLocation(placement, _modelServices->Logger(), _modelServices);
				TDF_Label aLabel = The_ShapeTool()->AddComponent(Ref(), subAssembly, location);
				SetName(aLabel, subAssemblyName);
				SetName(subAssembly, subAssemblyName);
				return gcnew BRepDocumentItem(subAssembly, _modelServices);
			}

			IXBRepDocumentItem^ BRepDocumentItem::AddAssembly(System::String^ subAssemblyName, XbimMatrix3D transform)
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
				return gcnew BRepDocumentItem(subAssemblyLabel, _modelServices);
			}

			IXBRepDocumentItem^ BRepDocumentItem::AddComponent(System::String^ name, IXBRepDocumentItem^ component, IIfcObjectPlacement^ objPlacement, ILogger^ logger)
			{

				TDF_Label label = static_cast<BRepDocumentItem^>(component)->Ref();
				//add the shape to the parent to avoid having to call update assemblies
				TDF_Label componentLabel = The_ShapeTool()->AddComponent(Ref(), label, XbimConvert::ToLocation(objPlacement, logger,_modelServices));
				SetName(componentLabel, name);
				BRepDocumentItem^ shapeRepNode = gcnew BRepDocumentItem(componentLabel, _modelServices);
				shapeRepNode->IsShapeRepresentation = true;
				return shapeRepNode;

			}

			IXBRepDocumentItem^ BRepDocumentItem::AddComponent(System::String^ name, int shapeId, IXShape^ shape)
			{
				return AddComponent(name, shapeId, shape, nullptr, nullptr);
			}


			IXBRepDocumentItem^ BRepDocumentItem::AddComponent(System::String^ name, IXBRepDocumentItem^ componentItem, XbimMatrix3D transform)
			{
				if (!(this->IsAssembly || this->IsSimpleShape)) throw gcnew XbimGeometryException("Components can only be added to Assemblies or simple shapes");
				//add the shape to the parent to avoid having to call update assemblies
				TopoDS_Shape mainShape = The_ShapeTool()->GetShape(Ref());
				gp_Trsf occTrans = XbimConvert::ToTransform(transform);
				TDF_Label storageLabel = static_cast<BRepDocumentItem^>(componentItem)->Ref();
				TopLoc_Location occLoc(occTrans);
				TDF_Label	aLabel = The_ShapeTool()->AddComponent(Ref(), storageLabel, occLoc);
				SetName(aLabel, name);
				return gcnew BRepDocumentItem(aLabel, _modelServices);
			}

			IXBRepDocumentItem^ BRepDocumentItem::AddComponent(System::String^ name, int shapeId, IXShape^ shape, IIfcObjectPlacement^ objPlacement, ILogger^ logger)
			{
				if (!(this->IsAssembly || this->IsSimpleShape)) throw gcnew XbimGeometryException("Components can only be added to Assemblies or simple shapes");

				const TopoDS_Shape& topoShape = static_cast<XShape^>(shape)->GetTopoShape();

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
					location = location.Multiplied(XbimConvert::ToLocation(objPlacement, logger, _modelServices));
				}
				componentLabel = The_ShapeTool()->AddComponent(Ref(), componentLabel, location);
				SetName(componentLabel, name);
				return gcnew BRepDocumentItem(componentLabel, _modelServices);


			}

			IXBRepDocumentItem^ BRepDocumentItem::AddSubShape(System::String^ name, IXShape^ shape, bool isExistingPart)
			{
				//first ensure the shape of the label has this shape in its compounds
				const TopoDS_Shape& subShape =  static_cast<XShape^>(shape)->GetTopoShape();

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
					return gcnew BRepDocumentItem(addedSubShape, _modelServices);
				}
				else
					throw gcnew System::Exception("Shape is not a sub shape of the selected assembly");

			}

			int BRepDocumentItem::IfcId::get()
			{
				return FlexShape::IfcId(Ref());
			}

			void BRepDocumentItem::IfcId::set(int id)
			{
				SetIntAttribute("IfcId", id);
			}
			short BRepDocumentItem::IfcTypeId::get()
			{
				return FlexShape::IfcTypeId(Ref());
			}

			void BRepDocumentItem::IfcTypeId::set(short id)
			{
				SetIntAttribute("IfcTypeId", id);
			}
			int BRepDocumentItem::NbMaterialLayers::get()
			{
				return GetIntAttribute("NbMaterialLayers");
			}

			void BRepDocumentItem::NbMaterialLayers::set(int nbMaterialLayers)
			{
				SetIntAttribute("NbMaterialLayers", nbMaterialLayers);
			}


			System::String^ BRepDocumentItem::IfcName::get()
			{
				return GetStringAttribute("IfcName");
			}

			void BRepDocumentItem::IfcName::set(System::String^ name)
			{
				SetStringAttribute("IfcName", name);
			}
			System::String^ BRepDocumentItem::IfcType::get()
			{
				return GetStringAttribute("IfcType");
			}

			void BRepDocumentItem::IfcType::set(System::String^ name)
			{
				SetStringAttribute("IfcType", name);
			}

			System::String^ BRepDocumentItem::Classification::get()
			{
				return GetStringAttribute("Classification");
			}

			void BRepDocumentItem::Classification::set(System::String^ classification)
			{
				SetStringAttribute("Classification", classification);
			}

			bool BRepDocumentItem::HasOpenings::get()
			{
				Handle(TDataStd_BooleanArray)features;
				bool hasFeatures = Ref().FindAttribute(TDataStd_BooleanArray::GetID(), features);
				if (hasFeatures) return features->Value(1); else return false;
			}
			void BRepDocumentItem::HasOpenings::set(bool hasOpenings)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(1, hasOpenings);
			}
			bool BRepDocumentItem::IsProduct::get()
			{
				return FlexShape::IsProduct(Ref());
			}
			void BRepDocumentItem::IsProduct::set(bool isProduct)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(3, isProduct);
			}

			bool BRepDocumentItem::IsShapeRepresentation::get()
			{
				return FlexShape::IsShapeRepresentation(Ref());
			}

			void BRepDocumentItem::IsShapeRepresentation::set(bool isShapeRepresentation)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(4, isShapeRepresentation);
			}

			bool BRepDocumentItem::IsShapeRepresentationMap::get()
			{
				return FlexShape::IsShapeRepresentationMap(Ref());
			}

			void BRepDocumentItem::IsShapeRepresentationMap::set(bool isShapeRepresentationMap)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(6, isShapeRepresentationMap);
			}

			bool BRepDocumentItem::IsGeometricRepresentationItem::get()
			{
				return FlexShape::IsGeometricRepresentationItem(Ref());
			}
			void BRepDocumentItem::IsGeometricRepresentationItem::set(bool isShapeRepresentation)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(5, isShapeRepresentation);
			}

			bool BRepDocumentItem::HasProjections::get()
			{
				Handle(TDataStd_BooleanArray)features;
				bool hasFeatures = Ref().FindAttribute(TDataStd_BooleanArray::GetID(), features);
				if (hasFeatures) return features->Value(2); else return false;
			}
			void BRepDocumentItem::HasProjections::set(bool hasProjections)
			{
				Handle(TDataStd_BooleanArray)features = TDataStd_BooleanArray::Set(Ref(), 1, 6);
				features->SetValue(2, hasProjections);
			}
			System::String^ BRepDocumentItem::GetStringAttribute(System::String^ attributeName)
			{
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);
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
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);
				}

			}

			void BRepDocumentItem::SetStringAttribute(System::String^ attributeName, System::String^ attributeValue)
			{
				if (System::String::IsNullOrWhiteSpace(attributeName) || System::String::IsNullOrWhiteSpace(attributeValue)) return;
				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);
				System::IntPtr pAttributeValue = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeValue);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					const char* pAnsiValue = static_cast<const char*>(pAttributeValue.ToPointer());
					namedData->SetString(pAnsiName, pAnsiValue);
				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeValue);
				}

			}

			int BRepDocumentItem::GetIntAttribute(System::String^ attributeName)
			{
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);
				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					return FlexShape::GetIntAttribute(Ref(), pAnsiName);
				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);
				}

			}

			void BRepDocumentItem::SetIntAttribute(System::String^ attributeName, int attributeValue)
			{
				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);

				try
				{
					const char* pAnsiName = static_cast<const char*>(pAttributeName.ToPointer());
					namedData->SetInteger(pAnsiName, attributeValue);

				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);

				}
			}

			System::Nullable<double> BRepDocumentItem::GetDoubleAttribute(System::String^ attributeName)
			{
				Handle(TDataStd_NamedData) attributes;
				bool hasAttributes = Ref().FindAttribute(TDataStd_NamedData::GetID(), attributes);
				if (!hasAttributes) return 0;
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);
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
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);
				}
			}

			int BRepDocumentItem::NbComponents::get()
			{
				return The_ShapeTool()->NbComponents(Ref());
			}

			IEnumerable<IXBRepDocumentItem^>^ BRepDocumentItem::Components::get()
			{
				TDF_LabelSequence labels;
				bool found = The_ShapeTool()->GetComponents(Ref(), labels);
				if (!found) return Enumerable::Empty<IXBRepDocumentItem^>();
				List<IXBRepDocumentItem^>^ components = gcnew List<IXBRepDocumentItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					components->Add(gcnew BRepDocumentItem(*it, _modelServices));
				return components;
			}

			IEnumerable<IXShape^>^ BRepDocumentItem::SubShapes::get()
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

			IEnumerable<IXBRepDocumentItem^>^ BRepDocumentItem::SubShapeStorageItems::get()
			{
				TDF_LabelSequence labels;
				bool found = The_ShapeTool()->GetSubShapes(Ref(), labels);
				if (!found) return Enumerable::Empty<IXBRepDocumentItem^>();
				List<IXBRepDocumentItem^>^ components = gcnew List<IXBRepDocumentItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					components->Add(gcnew BRepDocumentItem(*it, _modelServices));
				return components;
			}

			IXBRepDocumentItem^ BRepDocumentItem::ReferredShape::get()
			{
				TDF_Label refLabel;
				bool found = The_ShapeTool()->GetReferredShape(Ref(), refLabel);
				if (!found || refLabel.IsNull()) return nullptr;
				else
					return gcnew BRepDocumentItem(refLabel, _modelServices);
			}
			void BRepDocumentItem::SetDoubleAttribute(System::String^ attributeName, System::Nullable<double> attributeValue)
			{

				Handle(TDataStd_NamedData) namedData = TDataStd_NamedData::Set(Ref());
				System::IntPtr pAttributeName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(attributeName);
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
					System::Runtime::InteropServices::Marshal::FreeHGlobal(pAttributeName);

				}

			}
			TopoDS_Shape BRepDocumentItem::GetShape()
			{
				return The_ShapeTool()->GetShape(Ref());
			}
			void BRepDocumentItem::SetShape(const TopoDS_Shape shape)
			{
				return The_ShapeTool()->SetShape(Ref(), shape);
			}


			IXShape^ BRepDocumentItem::Shape::get()
			{
				TopoDS_Shape topoShape = The_ShapeTool()->GetShape(Ref());
				return ShapeFactory::GetXbimShape(topoShape);
			}
			void BRepDocumentItem::Shape::set(IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				The_ShapeTool()->SetShape(Ref(), topoShape);

			}


			System::Nullable<double> BRepDocumentItem::Volume::get()
			{

				return GetDoubleAttribute("Volume");
			}
			void BRepDocumentItem::Volume::set(System::Nullable<double> volume)
			{
				SetDoubleAttribute("Volume", volume);
			}
			System::Nullable<double> BRepDocumentItem::Area::get()
			{
				return GetDoubleAttribute("Area");
			}
			void BRepDocumentItem::Area::set(System::Nullable<double> area)
			{
				SetDoubleAttribute("Area", area);
			}
			System::Nullable<double> BRepDocumentItem::HeightMax::get()
			{
				return GetDoubleAttribute("HeightMax");
			}
			void BRepDocumentItem::HeightMax::set(System::Nullable<double> heightMax)
			{
				SetDoubleAttribute("HeightMax", heightMax);
			}
			System::Nullable<double> BRepDocumentItem::HeightMin::get()
			{
				return GetDoubleAttribute("HeightMin");
			}
			void BRepDocumentItem::HeightMin::set(System::Nullable<double> heightMin)
			{
				SetDoubleAttribute("HeightMin", heightMin);
			}

			System::Nullable<double> BRepDocumentItem::ThicknessMax::get()
			{
				return GetDoubleAttribute("ThicknessMax");
			}
			void BRepDocumentItem::ThicknessMax::set(System::Nullable<double> thickness)
			{
				SetDoubleAttribute("ThicknessMax", thickness);
			}

			System::String^ BRepDocumentItem::Key::get()
			{
				TCollection_AsciiString entry;
				TDF_Tool::Entry(Ref(), entry);
				return gcnew System::String(entry.ToCString());

			}

			System::String^ BRepDocumentItem::Name::get()
			{
				Handle(TDataStd_Name) name;
				bool hasName = Ref().FindAttribute(TDataStd_Name::GetID(), name);
				if (hasName)
					return gcnew System::String(name->Get().ToWideString());
				return
					System::String::Empty;
			}

			void BRepDocumentItem::Name::set(System::String^ name)
			{
				SetName(Ref(), name);
			}
			void BRepDocumentItem::SetName(const TDF_Label& label, System::String^ name)
			{
				if (!label.IsNull() && name != nullptr)
				{
					System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
					try
					{
						const char* pAnsi = static_cast<const char*>(p.ToPointer());
						TDataStd_Name::Set(label, pAnsi);
					}
					finally
					{
						System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
					}
				}
			}

			IXVisualMaterial^ BRepDocumentItem::VisualMaterial::get()
			{
				TDF_Label  materialLabel;
				try
				{
					Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref());

					if (aMatTool->GetShapeMaterial(Ref(), materialLabel))
					{
						Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(materialLabel);
						return gcnew Xbim::Geometry::Visual::VisualMaterial(aMat, gcnew BRepDocumentItem(materialLabel, _modelServices));
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

			void BRepDocumentItem::VisualMaterial::set(IXVisualMaterial^ visMaterial)
			{
				auto aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref());
				if (visMaterial->Label == nullptr) //need to add to the document
				{	
					//copy material in
					System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(visMaterial->Name);
					try
					{
						const char* pAnsi = static_cast<const char*>(p.ToPointer());
						auto materialLabel = aMatTool->AddMaterial(pAnsi);
						Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(materialLabel);
						XCAFDoc_VisMaterialCommon aMatCom = aMat->CommonMaterial();
						aMatCom.AmbientColor.SetValues(visMaterial->AmbientColor->Red, visMaterial->AmbientColor->Green, visMaterial->AmbientColor->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
						aMatCom.DiffuseColor.SetValues(visMaterial->DiffuseColor->Red, visMaterial->DiffuseColor->Green, visMaterial->DiffuseColor->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
						aMatCom.EmissiveColor.SetValues(visMaterial->EmissiveColor->Red, visMaterial->EmissiveColor->Green, visMaterial->EmissiveColor->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
						aMatCom.SpecularColor.SetValues(visMaterial->SpecularColor->Red, visMaterial->SpecularColor->Green, visMaterial->SpecularColor->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
						aMatCom.Shininess = visMaterial->Shininess;
						aMatCom.Transparency = visMaterial->Transparency;
						aMatCom.IsDefined = true;
						aMat->SetCommonMaterial(aMatCom);
						XCAFDoc_VisMaterialPBR pbr = aMat->ConvertToPbrMaterial();
						aMat->SetPbrMaterial(pbr);
						aMatTool->SetShapeMaterial(Ref(), materialLabel);
					}
					finally
					{
						System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
					}
				}
				else
				{
					BRepDocumentItem^ materialItem = dynamic_cast<BRepDocumentItem^>(visMaterial->Label);
					aMatTool->SetShapeMaterial(Ref(), materialItem->Ref());
				}
			}
			IEnumerable<IXBRepDocumentItem^>^ BRepDocumentItem::ShapesAssignedToMaterial::get()
			{
				TDF_LabelSequence userLabels;
				int numUsers = The_ShapeTool()->GetUsers(Ref(), userLabels, true);
				List<IXBRepDocumentItem^>^ users = gcnew List<IXBRepDocumentItem^>(numUsers);
				Handle(TDataStd_TreeNode) tree;
				bool hasTree = Ref().FindAttribute(XCAFDoc::VisMaterialRefGUID(), tree);
				if (hasTree)
				{
					int nbKids = tree->NbChildren(true);
					Handle(TDataStd_TreeNode) f = tree->First();
					for (int i = 0; i < nbKids; i++)
					{
						users->Add(gcnew BRepDocumentItem(f->Label(), _modelServices));
						f = f->Next();
					}
				}
				return users;
			}

		}
	}
}