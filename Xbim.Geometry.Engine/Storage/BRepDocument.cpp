
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <RWGltf_CafWriter.hxx>


#include <XCAFDoc_VisMaterialTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <TDF_Tool.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <RWGltf_CafWriter.hxx>
#include <TDataStd_Name.hxx>
#include <OSD_OpenFile.hxx>

#include "BRepDocument.h"
#include "StorageItem.h"
#include "../Visual/VisualMaterial.h"
#include "../Factories//ShapeFactory.h"
#include "../Exceptions//XbimGeometryServiceException.h"
#include "Unmanaged/NBRepDocument.h"

#include "./Unmanaged/WexBim_CafWriter.h"

#include "../BRep/XShape.h"

#include "../Services/MeshFactors.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Factories;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Visual;

using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			IXStorageItem^ BRepDocument::CreateAssembly(System::String^ name)
			{

				TDF_Label assemblyLabel = XCAFDoc_ShapeTool()->NewShape();
				StorageItem::SetName(assemblyLabel, name);
				return gcnew StorageItem(assemblyLabel);

			}

			//srl this might need to be obsoleted
			IXStorageItem^ BRepDocument::CreateAssembly(System::String^ name, IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				TDF_Label assemblyLabel = XCAFDoc_ShapeTool()->AddShape(topoShape);
				StorageItem::SetName(assemblyLabel, name);
				return gcnew StorageItem(assemblyLabel);
			}



			bool BRepDocument::ExportGltf(System::String^ path, bool binary)
			{
				System::IntPtr p = Marshal::StringToHGlobalAnsi(path);
				try
				{
					const char* pAnsi = static_cast<const char*>(p.ToPointer());
					RWGltf_CafWriter gltfWriter(pAnsi, binary);
					TColStd_IndexedDataMapOfStringString fileInfo;
					fileInfo.Add("Author", "Xbim");
					fileInfo.Add("Exporter", "Flex");
					Message_ProgressRange mp;
					bool success = gltfWriter.Perform(Ref(), fileInfo, mp);
					return success;
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
			}
			bool BRepDocument::ExportSTEP(System::String^ path)
			{
				System::IntPtr p = Marshal::StringToHGlobalAnsi(path);
				try
				{
					const char* pAnsi = static_cast<const char*>(p.ToPointer());
					STEPCAFControl_Writer writer;
					writer.SetColorMode(true);
					bool success = writer.Perform(Ref(), pAnsi);
					return success;
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
			}
			array<System::Byte>^ BRepDocument::ExportWexbim(IXColourRGB^ defaultColour, MeshGranularity meshGranularity)
			{
				std::stringstream output;
				try
				{
					Quantity_ColorRGBA defaultRGBA((float)defaultColour->Red, (float)defaultColour->Green, (float)defaultColour->Blue, 1.0);
					//sort out mesh parameters
					MeshFactors^ meshFactors = gcnew MeshFactors(ConversionFactorForOneMeter.Value, PrecisionFactor.Value);
					meshFactors->SetGranularity(meshGranularity);
					WexBim_CafWriter writer(defaultRGBA, meshFactors->OneMeter, meshFactors->LinearDefection, meshFactors->AngularDeflection, meshFactors->Tolerance);
					TColStd_IndexedDataMapOfStringString fileInfo;
					fileInfo.Add("Author", "Xbim");
					fileInfo.Add("Exporter", "Flex");
					bool success = writer.Perform(Ref(), output, fileInfo);
					if (!success)
						Standard_Failure::Raise("General failure creating WexBim content");
					int size = (int) output.str().length();
					auto buffer = std::make_unique<char[]>(size);
					output.seekg(0);
					output.read(buffer.get(), size);
					cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
					Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
					return byteArray;
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew IOException(msg);
				}
			}

			bool BRepDocument::ExportWexbim(System::String^ path, IXColourRGB^ defaultColour, MeshGranularity meshGranularity)
			{
				System::IntPtr p = Marshal::StringToHGlobalAnsi(path);
				try
				{
					//create a file				
					const char* pAnsiPath = static_cast<const char*>(p.ToPointer());
					std::ofstream wexBimFile;
					OSD_OpenStream(wexBimFile, pAnsiPath, std::ios::out | std::ios::binary);
					if (!wexBimFile.is_open()
						|| !wexBimFile.good())
					{
						std::string msg("File '");
						msg += pAnsiPath;
						msg += "' can not be created";
						Standard_Failure::Raise(msg.c_str());
						return false;
					}
					Quantity_ColorRGBA defaultRGBA((float)defaultColour->Red, (float)defaultColour->Green, (float)defaultColour->Blue, 1.0);
					//sort out mesh parameters
					MeshFactors^ meshFactors = gcnew MeshFactors(ConversionFactorForOneMeter.Value, PrecisionFactor.Value);
					meshFactors->SetGranularity(meshGranularity);
					WexBim_CafWriter writer(defaultRGBA, meshFactors->OneMeter, meshFactors->LinearDefection, meshFactors->AngularDeflection, meshFactors->Tolerance);
					TColStd_IndexedDataMapOfStringString fileInfo;
					fileInfo.Add("Author", "Xbim");
					fileInfo.Add("Exporter", "Flex");
					bool success = writer.Perform(Ref(), wexBimFile, fileInfo);

					return success;
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew IOException(msg);
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
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
					return gcnew VisualMaterial(aMat, gcnew StorageItem(materialLabel));
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
			}

			IXStorageItem^ BRepDocument::AddShape(int id, IXShape^ shape)
			{
				//if (shape == nullptr) return nullptr;
				const TopoDS_Shape& s = static_cast<XShape^>(shape)->GetTopoShape();
				TDF_Label label = XCAFDoc_ShapeTool()->AddShape(s, false, false);
				StorageItem::SetName(label, System::Convert::ToString(id));
				return gcnew StorageItem(label);
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




			void BRepDocument::SetMaterial(IXShape^ shape, IXVisualMaterial^ visMaterial)
			{
				VisualMaterial^ visMatItem = dynamic_cast<VisualMaterial^>(visMaterial);
				if (visMatItem != nullptr)
				{
					StorageItem^ storeItem = dynamic_cast<StorageItem^>(visMaterial->Label);
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
					materials->Add(gcnew VisualMaterial(aMat, gcnew StorageItem(*it)));
				}
				return materials;
			}
			IEnumerable<IXStorageItem^>^ BRepDocument::GetMaterials()
			{
				Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(Ref()->Main());
				TDF_LabelSequence materialLabels;
				aMatTool->GetMaterials(materialLabels);
				List<IXStorageItem^>^ materials = gcnew List<IXStorageItem^>(materialLabels.Size());
				for (auto& it = materialLabels.cbegin(); it != materialLabels.cend(); ++it)
				{
					materials->Add(gcnew StorageItem(*it));
				}
				return materials;
			}



			bool BRepDocument::RemoveAssembly(IXStorageItem^ assembly)
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



			IXVisualMaterial^ BRepDocument::GetMaterial(IXStorageItem^ item)
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
								return gcnew VisualMaterial(aMat, gcnew StorageItem(materialLabel));
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
					return gcnew VisualMaterial(aMat, gcnew StorageItem(materialLabel));
				}

				return nullptr;
			}
			void BRepDocument::UpdateAssemblies()
			{
				Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				shapeTool->UpdateAssemblies();
			}



			IXStorageItem^ BRepDocument::RootItem::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				return gcnew StorageItem(myAssembly->BaseLabel());
			}

			IEnumerable<IXStorageItem^>^ BRepDocument::FreeShapes::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				TDF_LabelSequence labels;
				myAssembly->GetFreeShapes(labels);
				if (labels.IsEmpty()) return Enumerable::Empty<IXStorageItem^>();
				List<IXStorageItem^>^ freeShapes = gcnew List<IXStorageItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					freeShapes->Add(gcnew StorageItem(*it));
				return freeShapes;
			}
			IEnumerable<IXStorageItem^>^ BRepDocument::Shapes::get()
			{
				Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Ref()->Main());
				TDF_LabelSequence labels;
				myAssembly->GetShapes(labels);
				if (labels.IsEmpty()) return Enumerable::Empty<IXStorageItem^>();
				List<IXStorageItem^>^ shapes = gcnew List<IXStorageItem^>(labels.Size());
				for (auto& it = labels.cbegin(); it != labels.cend(); ++it)
					shapes->Add(gcnew StorageItem(*it));
				return shapes;
			}



			void BRepDocument::LoadSTEP(System::String^ path)
			{
				System::IntPtr p = Marshal::StringToHGlobalAnsi(path);
				try
				{
					const char* pAnsi = static_cast<const char*>(p.ToPointer());
					STEPCAFControl_Reader stepReader;
					Message_ProgressRange mp;
					if (!stepReader.Perform(pAnsi, Ref(), mp))
						throw gcnew FileLoadException("Error loading file: " + path);
				}
				finally
				{
					Marshal::FreeHGlobal(p);
				}
			}
		}
	}
}