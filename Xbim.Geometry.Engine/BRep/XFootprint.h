#pragma once
#include "../XbimHandle.h"
#include "Unmanaged/NFootprint.h"
#include <TopoDS_Solid.hxx>
#include <Geom_Plane.hxx>


using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{


			public ref class XFootprint : XbimHandle<NFootprint>, IXFootprint
			{
			private:
				
				static unsigned char SfaByteOrder = 1; // Little Endian
				
			public:
				XFootprint() : XbimHandle(new NFootprint()) {};
				XFootprint(NFootprint* footprint) : XbimHandle(footprint) {};
				virtual property IEnumerable<IEnumerable<IXPolyLoop2d^>^>^ Bounds {IEnumerable<IEnumerable<IXPolyLoop2d^>^>^ get(); };
				virtual property double MinZ {double get() { return Ptr()->MinZ; }; };
				virtual property double MaxZ {double get() { return Ptr()->MaxZ; }; };
				virtual property bool IsClose {bool get() { return Ptr()->IsClose; };  };
				virtual property SFAGeometryType SfaGeometryType {SFAGeometryType get(); }
				std::vector<std::vector<Handle(Poly_Polygon2D)>> Polygon2Ds() { return Ptr()->Bounds; }
				/// <summary>
				/// Writes the footprint in SFA Well Known Binary Format
				/// </summary>
				/// <param name="binaryWriter"></param>
				virtual void Write(BinaryWriter^ binaryWriter);
				/// <summary>
				/// Writes the footprint in SFA Well Known Text Format
				/// </summary>
				/// <param name="textWriter"></param>
				virtual void Write(TextWriter^ textWriter);
				virtual System::String^ ToString() override;
			};
		}
	}
}