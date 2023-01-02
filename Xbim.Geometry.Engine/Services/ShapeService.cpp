#include "ShapeService.h"
#include "Unmanaged/NWexBimMesh.h"
#include "ModelGeometryService.h"
#include "../BRep//XShape.h"
#include <vector>


using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{

		namespace Services
		{

			IXShape^ ShapeService::Convert(System::String^ brepString)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			System::String^ ShapeService::Convert(IXShape^ shape)
			{
				return gcnew System::String("");
			}

			System::String^ ShapeService::Convert(IXbimGeometryObject^ v5Shape)
			{
				return gcnew System::String("");
			}

			IXbimGeometryObject^ ShapeService::ConvertToV5(System::String^ brepString)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Cut(IXShape^ body, IXShape^ subtraction)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Cut(IXShape^ body, IEnumerable<IXShape^>^ subtractions)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Transform(IXShape^ shape, XbimMatrix3D transformMatrix)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			void ShapeService::Triangulate(IXShape^ shape)
			{
				throw gcnew System::NotImplementedException();
			}

			IXShape^ ShapeService::RemovePlacement(IXShape^ shape)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::SetPlacement(IXShape^ shape, IIfcObjectPlacement^ placement)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::UnifyDomain(IXShape^ shape)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Union(IXShape^ body, IXShape^ addition)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Union(IXShape^ body, IEnumerable<IXShape^>^ additions)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Moved(IXShape^ shape, IIfcObjectPlacement^ placement, bool invertPlacement)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Transform(IXShape^ shape, IXMatrix^ transform)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Intersect(IXShape^ shape, IXShape^ intersect)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Intersect(IXShape^ shape, IEnumerable<IXShape^>^ intersect)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Moved(IXShape^ shape, IXLocation^ location)
			{
				TopoDS_Shape topoShape = TOPO_SHAPE(shape);
				auto xLoc = static_cast<XLocation^>(location);
				topoShape.Move(xLoc->Ref());
				return  XShape::GetXbimShape(topoShape);
			}

			IXShape^ ShapeService::Scaled(IXShape^ shape, double scale)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Combine(IXShape^ shape, IEnumerable<IXShape^>^ intersect)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			bool ShapeService::IsFacingAwayFrom(IXFace^ face, IXDirection^ direction)
			{
				return false;
			}

			IXbimGeometryObject^ ShapeService::ConvertToV5(IXShape^ shape)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ ShapeService::Combine(IEnumerable<IXShape^>^ shapes)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			array<System::Byte>^ ShapeService::CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves)
			{
				return CreateWexBimMesh(shape, meshFactors, scale, bounds, hasCurves,false,true,true);
			}

			array<System::Byte>^ ShapeService::CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds)
			{
				bool hasCurves;
				return CreateWexBimMesh(shape, meshFactors, scale, bounds, hasCurves);
			}

			array<System::Byte>^ ShapeService::CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves, bool checkEdges, bool cleanBefore, bool cleanAfter)
			{
				const TopoDS_Shape& topoShape = static_cast<XShape^>(shape)->GetTopoShape();
				auto mesh = NWexBimMesh::CreateMesh(topoShape, meshFactors->Tolerance, meshFactors->LinearDefection, meshFactors->AngularDeflection, scale);
				auto bMin = mesh.BndBox.CornerMin();
				auto bMax = mesh.BndBox.CornerMax();
				bounds = gcnew XAxisAlignedBox(Bnd_Box(gp_Pnt(bMin[0], bMin[1], bMin[2]), gp_Pnt(bMax[0], bMax[1], bMax[2])));
				std::stringstream output;
				mesh.WriteToStream(output);
				int size = (int)output.str().length();
				auto buffer = std::make_unique<char[]>(size);
				output.seekg(0);
				output.read(buffer.get(), size);
				cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
				Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
				return byteArray;
			}

		}
	}
}