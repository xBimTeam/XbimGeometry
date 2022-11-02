#include "XLocation.h"

IXLocation^ Xbim::Geometry::BRep::XLocation::Multiplied(IXLocation^ location)
{
	auto xbimLoc = dynamic_cast<XLocation^>(location);
	if (location == nullptr || xbimLoc == nullptr || xbimLoc->IsIdentity) return gcnew XLocation(Ref());
	return gcnew XLocation(xbimLoc->Ref().Multiplied(Ref()));
}

