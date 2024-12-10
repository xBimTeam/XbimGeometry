#include "ShapeBinarySerializer.h"
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;


class membuf : public std::basic_streambuf<char> {
public:
	membuf(const uint8_t* p, size_t l) {
		setg((char*)p, (char*)p, (char*)p + l);
	}
};
class memstream : public std::istream {
public:
	memstream(const unsigned char* p, size_t l) :
		std::istream(&_buffer),
		_buffer(p, l) {
		rdbuf(&_buffer);
	}

private:
	membuf _buffer;
};

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ShapeBinarySerializer::ShapeBinarySerializer(Xbim::Geometry::Services::ModelGeometryService^ modelService) 
			{
				_modelService = modelService;
				_loggingService = modelService->LoggingService;
			};

			array<System::Byte>^ ShapeBinarySerializer::ToArray(IXShape^ shape, bool withTriangles, bool withNormals)
			{
				try
				{
					if (shape == nullptr) return  gcnew cli::array<System::Byte>(0);
					TopoDS_Shape topoShape = ((XShape^)shape)->GetTopoShape();
					std::stringstream output;
					BinTools::Write(topoShape, output, withTriangles, withNormals, BinTools_FormatVersion::BinTools_FormatVersion_VERSION_3); //version is fixed at 3, the most current at time of authoring
					int size = (int)output.str().length();

					auto buffer = std::make_unique<char[]>(size);
					output.seekg(0);
					output.read(buffer.get(), size);
					cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
					System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
					return byteArray;
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Shape serialization error, " + msg);
				}
			}

			IXShape^ ShapeBinarySerializer::FromArray(array<System::Byte>^ bytes)
			{
				TopoDS_Shape topoShape;

				try
				{

					pin_ptr<System::Byte> pbArr = &bytes[0];
					memstream iStrm(pbArr, bytes->Length);
					BinTools::Read(topoShape, iStrm);
					return XShape::GetXbimShape(topoShape);
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Shape deserialization error, " + msg);
				}


			}
		}
	}
}