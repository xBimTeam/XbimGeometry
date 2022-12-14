

#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Application.hxx>

#include <XCAFDoc_VisMaterialTool.hxx>

#include "./Unmanaged/NBRepDocument.h"
#include <TDF_Tool.hxx>

#include <TDataStd_Name.hxx>
#include <OSD_OpenFile.hxx>

#include "BRepDocument.h"
#include "BRepDocumentItem.h"
#include "../Visual/VisualMaterial.h"
#include "../Factories//ShapeFactory.h"
#include "../Exceptions//XbimGeometryServiceException.h"


#include "../BRep/XShape.h"

#include "../Services/MeshFactors.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Factories;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Visual;

using namespace Xbim::Geometry::Exceptions;
#define XCAFDoc_ShapeTool() XCAFDoc_DocumentTool::ShapeTool(Ref()->Main())
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			IXBRepDocumentItem^ BRepDocument::CreateAssembly(System::String^ name)
			{
				
				TDF_Label assemblyLabel = XCAFDoc_ShapeTool()->NewShape();
				BRepDocumentItem::SetName(assemblyLabel, name);
				return gcnew BRepDocumentItem(assemblyLabel);

			}

			//srl this might need to be obsoleted
			IXBRepDocumentItem^ BRepDocument::CreateAssembly(System::String^ name, IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				TDF_Label assemblyLabel = XCAFDoc_ShapeTool()->AddShape(topoShape);
				BRepDocumentItem::SetName(assemblyLabel, name);
				return gcnew BRepDocumentItem(assemblyLabel);
			}



			IXBRepDocumentItem^ BRepDocument::AddShape(int id, IXShape^ shape)
			{
				//if (shape == nullptr) return nullptr;
				const TopoDS_Shape& s = static_cast<XShape^>(shape)->GetTopoShape();
				TDF_Label label = XCAFDoc_ShapeTool()->AddShape(s, false, false);
				BRepDocumentItem::SetName(label, System::Convert::ToString(id));
				return gcnew BRepDocumentItem(label);
			}

			IXShape^ BRepDocument::GetShape(int id)
			{
				TDF_Label label;
				TDF_Tool::Label(Ref()->GetData(), std::to_string(id).c_str(), label);
				TopoDS_Shape topoShape = XCAFDoc_ShapeTool()->GetShape(label);
				if (topoShape.IsNull())
					return nullptr;
				else
					return ShapeFactory::GetXbimShape(topoShape);
			}


			bool BRepDocument::RemoveAssembly(IXBRepDocumentItem^ assembly)
			{
				System::IntPtr pShape = Marshal::StringToHGlobalAnsi(assembly->Key);
				try
				{
					const char* pShapeAnsi = static_cast<const char*>(pShape.ToPointer());
					TDF_Label aShapeLabel;
					TDF_Tool::Label(Ref()->GetData(), pShapeAnsi, aShapeLabel);
					if (!aShapeLabel.IsNull())
					{
						Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(aShapeLabel);
						//return RemoveComponents(aShapeLabel);
						return shapeTool->RemoveShape(aShapeLabel, true);
					}
				}
				finally
				{
					Marshal::FreeHGlobal(pShape);
				}
				return false;
			}

			bool BRepDocument::RemoveComponents(const TDF_Label& aShapeLabel)
			{
				Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(aShapeLabel);
				TDF_LabelSequence labels;
				bool hasComponents = XCAFDoc_ShapeTool::GetComponents(aShapeLabel, labels, true);
				if (hasComponents)
				{
					for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					{

						TDF_Label referredLabel;
						bool hasReferredShape = XCAFDoc_ShapeTool::GetReferredShape(*it, referredLabel);
						if (hasReferredShape)
						{
							TDF_LabelSequence referringLabels;
							if (XCAFDoc_ShapeTool::GetUsers(referredLabel, referringLabels) == 1) //only remove if nothing else is using
								RemoveComponents(referredLabel);
						}
						shapeTool->RemoveShape(*it, true);
					}
				}
				return shapeTool->RemoveShape(aShapeLabel, true);
			}


			void BRepDocument::UpdateAssemblies()
			{
				Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				shapeTool->UpdateAssemblies();
			}

			IXBRepDocumentItem^ BRepDocument::RootItem::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				return gcnew BRepDocumentItem(myAssembly->BaseLabel());
			}

			IEnumerable<IXBRepDocumentItem^>^ BRepDocument::FreeShapes::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				TDF_LabelSequence labels;
				myAssembly->GetFreeShapes(labels);
				if (labels.IsEmpty()) return Enumerable::Empty<IXBRepDocumentItem^>();
				List<IXBRepDocumentItem^>^ freeShapes = gcnew List<IXBRepDocumentItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					freeShapes->Add(gcnew BRepDocumentItem(*it));
				return freeShapes;
			}
			IEnumerable<IXBRepDocumentItem^>^ BRepDocument::Shapes::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				TDF_LabelSequence labels;
				myAssembly->GetShapes(labels);
				if (labels.IsEmpty()) return Enumerable::Empty<IXBRepDocumentItem^>();
				List<IXBRepDocumentItem^>^ shapes = gcnew List<IXBRepDocumentItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					shapes->Add(gcnew BRepDocumentItem(*it));
				return shapes;
			}	


			IXVisualMaterial^ BRepDocument::AddVisualMaterial(IXVisualMaterial^ visMaterial)
			{


				System::IntPtr p = Marshal::StringToHGlobalAnsi(visMaterial->Name);
				try
				{
					const char* pAnsi = static_cast<const char*>(p.ToPointer());

					Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
					TDF_Label materialLabel = NBRepDocument::FindVisualMaterial(Ref(), pAnsi);
					if (materialLabel.IsNull())
						materialLabel = aMatTool->AddMaterial(pAnsi);
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
					return gcnew VisualMaterial(aMat, gcnew BRepDocumentItem(materialLabel));
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
			}

			void BRepDocument::SetMaterial(IXShape^ shape, IXVisualMaterial^ visMaterial)
			{
				VisualMaterial^ visMatItem = dynamic_cast<VisualMaterial^>(visMaterial);
				if (visMatItem != nullptr)
				{
					BRepDocumentItem^ storeItem = dynamic_cast<BRepDocumentItem^>(visMaterial->Label);
					if (storeItem != nullptr)
					{
						Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
						const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
						aMatTool->SetShapeMaterial(topoShape, storeItem->Ref());
					}
				}
			}


			IEnumerable<IXVisualMaterial^>^ BRepDocument::GetVisualMaterials()
			{
				Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
				TDF_LabelSequence materialLabels;
				aMatTool->GetMaterials(materialLabels);
				List<IXVisualMaterial^>^ materials = gcnew List<IXVisualMaterial^>(materialLabels.Size());
				for (auto& it = materialLabels.cbegin(); it != materialLabels.cend(); ++it)
				{
					Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(*it);
					materials->Add(gcnew VisualMaterial(aMat, gcnew BRepDocumentItem(*it)));
				}
				return materials;
			}
			IEnumerable<IXBRepDocumentItem^>^ BRepDocument::GetMaterials()
			{
				Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
				TDF_LabelSequence materialLabels;
				aMatTool->GetMaterials(materialLabels);
				List<IXBRepDocumentItem^>^ materials = gcnew List<IXBRepDocumentItem^>(materialLabels.Size());
				for (auto& it = materialLabels.cbegin(); it != materialLabels.cend(); ++it)
				{
					materials->Add(gcnew BRepDocumentItem(*it));
				}
				return materials;
			}

			IXVisualMaterial^ BRepDocument::GetMaterial(IXBRepDocumentItem^ item)
			{
				if (!item->IsNull)
				{
					System::IntPtr pShape = Marshal::StringToHGlobalAnsi(item->Key);
					try
					{

						const char* pShapeAnsi = static_cast<const char*>(pShape.ToPointer());
						TDF_Label aShapeLabel;
						TDF_Tool::Label(Ref()->GetData(), pShapeAnsi, aShapeLabel);
						Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
						if (!aShapeLabel.IsNull())
						{
							TDF_Label  materialLabel;
							if (aMatTool->GetShapeMaterial(aShapeLabel, materialLabel))
							{
								Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(materialLabel);
								return gcnew VisualMaterial(aMat, gcnew BRepDocumentItem(materialLabel));
							}
						}
					}
					finally
					{
						Marshal::FreeHGlobal(pShape);
					}
				}
				return nullptr;
			}
			IXVisualMaterial^ BRepDocument::GetMaterial(IXShape^ shape)
			{

				Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());

				TDF_Label  materialLabel;
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				if (aMatTool->GetShapeMaterial(topoShape, materialLabel))
				{
					Handle(XCAFDoc_VisMaterial) aMat = aMatTool->GetMaterial(materialLabel);
					return gcnew VisualMaterial(aMat, gcnew BRepDocumentItem(materialLabel));
				}

				return nullptr;
			}


		}
	}
}