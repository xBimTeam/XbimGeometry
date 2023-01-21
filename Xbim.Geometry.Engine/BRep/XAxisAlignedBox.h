#pragma once
#pragma once
#include "../XbimHandle.h"
#include<Bnd_Box.hxx>

using namespace Xbim::Geometry::Abstractions;


namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XAxisAlignedBox : XbimHandle<Bnd_Box>, IXAxisAlignedBoundingBox
			{
			public:
				XAxisAlignedBox(const Bnd_Box& box) : XbimHandle(new Bnd_Box(box)) { };
				XAxisAlignedBox(Bnd_Box* pBox) : XbimHandle(pBox) { };
				XAxisAlignedBox() : XbimHandle(new Bnd_Box()) { Ref().SetVoid(); };
				XAxisAlignedBox(double x, double y, double z, double sizeX, double sizeY, double sizeZ) : XbimHandle(new Bnd_Box(gp_Pnt(x, y, z), gp_Pnt(x + sizeX, y + sizeY, z + sizeZ))) { };

				virtual property IXPoint^ CornerMin {IXPoint^ get(); };
				virtual property IXPoint^ CornerMax {IXPoint^ get(); };
				virtual property System::String^ Json {System::String^ get(); };
				virtual property double LenX {double get(); };
				virtual property double LenY {double get(); };
				virtual property double LenZ {double get(); };
				virtual property double Gap {double get(); };
				virtual property bool IsVoid { bool get(); };
				virtual IXAxisAlignedBoundingBox^ Union(IXAxisAlignedBoundingBox^ other);
				virtual property IXPoint^ Centroid {IXPoint^ get(); };
				virtual IXAxisAlignedBoundingBox^ Translated(double x, double y, double z);
				virtual IXAxisAlignedBoundingBox^ Translated(IXPoint^ translation);
			};

		}
	}
}
