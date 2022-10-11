

#include <BRepTools.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRep_Builder.hxx>

#include "XbimOccWriter.h"
using System::Runtime::InteropServices::Marshal;
using namespace System::Threading;
namespace Xbim 
{
	namespace Geometry
	{

		XbimOccWriter::XbimOccWriter()
		{
		}


		bool XbimOccWriter::Write(const TopoDS_Shape& shape, System::String^ filename)
		{
			
			Standard_CString fName = (const char*)(Marshal::StringToHGlobalAnsi(filename)).ToPointer();
			try
			{

				return (BRepTools::Write(shape, fName) == Standard_True);

			}
			catch (...)
			{
				return false;
			} 
			finally
			{
				Marshal::FreeHGlobal(System::IntPtr((void*)fName));
			}
			
		}

		bool XbimOccWriter::Write(IXbimSolidSet^ solidSet, System::String^ filename)
		{
			BRep_Builder b;
			TopoDS_Compound c;
			b.MakeCompound(c);
			for each (IXbimSolid^ var in solidSet)
			{
				XbimSolidV5^ xShape = dynamic_cast<XbimSolidV5^>(var);
				if (xShape == nullptr)
					throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
				b.Add(c, xShape);
			}
			return Write(c, filename);
		}

		bool XbimOccWriter::Write(IXbimSolid^ solid, System::String^ filename)
		{
			XbimSolidV5^ xSolid = dynamic_cast<XbimSolidV5^>(solid);
			if (xSolid == nullptr) throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			return Write((const TopoDS_Solid&)xSolid,filename);
		}

		bool XbimOccWriter::Write(IXbimFace^ face, System::String^ filename)
		{
			XbimFaceV5^ xFace = dynamic_cast<XbimFaceV5^>(face);
			if (xFace == nullptr) throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			return Write((const TopoDS_Face&)xFace, filename);
		}

		bool XbimOccWriter::Write(IXbimWire^ wire, System::String^ filename)
		{
			XbimWireV5^ xWire = dynamic_cast<XbimWireV5^>(wire);
			if (xWire == nullptr) throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			return Write((const TopoDS_Wire&)xWire, filename);
		}
		bool XbimOccWriter::Write(IXbimShell^ shell, System::String^ filename)
		{
			XbimShellV5^ xShell = dynamic_cast<XbimShellV5^>(shell);
			if (xShell == nullptr) throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			return Write((const TopoDS_Shell&)xShell, filename);
		}
		
		bool XbimOccWriter::Write(IXbimGeometryObjectSet^ objects, System::String^ filename)
		{
			XbimCompoundV5^ xCompound = dynamic_cast<XbimCompoundV5^>(objects);
			if (xCompound != nullptr) return Write((const TopoDS_Compound&)xCompound, filename);
			throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			
		}

		bool XbimOccWriter::Write(IXbimGeometryObject^ obj, System::String^ filename)
		{
			if (dynamic_cast<IXbimSolid^>(obj)) return Write((IXbimSolid^)obj, filename);
			else if (dynamic_cast<IXbimShell^>(obj)) return Write((IXbimShell^)obj, filename);
			else if (dynamic_cast<IXbimWire^>(obj)) return Write((IXbimWire^)obj, filename);
			else if (dynamic_cast<IXbimFace^>(obj)) return Write((IXbimFace^)obj, filename);
			else if (dynamic_cast<IXbimGeometryObjectSet^>(obj)) return Write((IXbimGeometryObjectSet^)obj, filename);
			else throw gcnew System::Exception("Only objects from Xbim.OCC namespace can be written using the Xbim OCC writer");
			
		}
	}
}