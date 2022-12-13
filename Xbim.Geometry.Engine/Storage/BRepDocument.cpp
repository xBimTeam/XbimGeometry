
#include <XCAFDoc_DocumentTool.hxx>
//#include <XCAFDoc_ShapeTool.hxx>
//#include <STEPCAFControl_Writer.hxx>
//#include <RWGltf_CafWriter.hxx>
//
//
//#include <XCAFDoc_VisMaterialTool.hxx>
//#include <XCAFDoc_ColorTool.hxx>
//#include <XCAFDoc_MaterialTool.hxx>
#include <TDF_Tool.hxx>

//#include <RWGltf_CafWriter.hxx>
#include <TDataStd_Name.hxx>
#include <OSD_OpenFile.hxx>

#include "BRepDocument.h"
#include "BRepDocumentItem.h"
#include "../Visual/VisualMaterial.h"
#include "../Factories//ShapeFactory.h"
#include "../Exceptions//XbimGeometryServiceException.h"
#

//#include "./Unmanaged/WexBim_CafWriter.h"

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
		}
	}
}