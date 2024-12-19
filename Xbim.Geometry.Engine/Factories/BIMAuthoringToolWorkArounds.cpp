#include "BIMAuthoringToolWorkArounds.h"
#include "GeometryFactory.h"
#include "ProfileFactory.h"

using namespace System::Text::RegularExpressions;
using namespace System::Linq;
bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::FixRevitIncorrectArcCentreSweptCurve(IIfcSurfaceOfLinearExtrusion^ surfaceOfLinearExtrusions, TopoDS_Edge& fixedEdge)
{
	if (!ShouldApplyRevitIncorrectArcCentreSweptCurve()) return false;
	if (surfaceOfLinearExtrusions->Position == nullptr) return false;
	IIfcArbitraryOpenProfileDef^ arbitraryOpenProfileDef = dynamic_cast<IIfcArbitraryOpenProfileDef^>(surfaceOfLinearExtrusions->SweptCurve);
	if (arbitraryOpenProfileDef == nullptr) return false; //not in scope
	IIfcTrimmedCurve^ tc = dynamic_cast<IIfcTrimmedCurve^>(arbitraryOpenProfileDef->Curve);
	if (tc == nullptr) return false; //its always a trimmed curve in this error
	IIfcCircle^ circle = dynamic_cast<IIfcCircle^>(tc->BasisCurve);
	if (circle == nullptr) return false; //always a circle

	//In this error the centre has been transformed twice, recalculate and update temporarily the centre
	//trim 1 and trim 2 will be only cartesian points in this error
	IIfcCartesianPoint^ trim1 = dynamic_cast<IIfcCartesianPoint^>(Enumerable::FirstOrDefault(tc->Trim1));
	IIfcCartesianPoint^ trim2 = dynamic_cast<IIfcCartesianPoint^>(Enumerable::FirstOrDefault(tc->Trim2));
	if (trim1 == nullptr || trim2 == nullptr) return false;

	gp_Pnt p1 = GEOMETRY_FACTORY->BuildPoint3d(trim1);
	gp_Pnt p2 = GEOMETRY_FACTORY->BuildPoint3d(trim2);

	double radsq = circle->Radius * circle->Radius;
	double qX = System::Math::Sqrt(((p2.X() - p1.X()) * (p2.X() - p1.X())) + ((p2.Y() - p1.Y()) * (p2.Y() - p1.Y())));
	double x3 = (p1.X() + p2.X()) / 2;
	double centreX = x3 - System::Math::Sqrt(radsq - ((qX / 2) * (qX / 2))) * ((p1.Y() - p2.Y()) / qX);
	double qY = System::Math::Sqrt(((p2.X() - p1.X()) * (p2.X() - p1.X())) + ((p2.Y() - p1.Y()) * (p2.Y() - p1.Y())));
	double y3 = (p1.Y() + p2.Y()) / 2;
	double centreY = y3 - System::Math::Sqrt(radsq - ((qY / 2) * (qY / 2))) * ((p2.X() - p1.X()) / qY);

	ITransaction^ txn = ModelGeometryService->Model->BeginTransaction("Fix Centre");
	IIfcPlacement^ p = dynamic_cast<IIfcPlacement^>(circle->Position);
	p->Location->Coordinates[0] = centreX;
	p->Location->Coordinates[1] = centreY;
	p->Location->Coordinates[2] = 0;
	fixedEdge = PROFILE_FACTORY->BuildProfileEdge(surfaceOfLinearExtrusions->SweptCurve);
	txn->RollBack();
	return true;
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::FixRevitSweptSurfaceExtrusionInFeet(gp_Vec& vec)
{
	if (!ShouldApplyRevitSweptSurfaceExtrusionInFeet()) return false;
	vec *= ModelGeometryService->OneFoot;
	return true;
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::FixRevitIncorrectBsplineSweptCurve(IIfcSurfaceOfLinearExtrusion^ surfaceOfLinearExtrusions, TopoDS_Edge& fixedEdge)
{
	IIfcArbitraryOpenProfileDef^ pDef = dynamic_cast<IIfcArbitraryOpenProfileDef^>(surfaceOfLinearExtrusions->SweptCurve);
	IIfcTrimmedCurve^ tc = nullptr;
	IIfcBSplineCurveWithKnots^ bspline = nullptr;
	if (pDef != nullptr)
	{
		tc = dynamic_cast<IIfcTrimmedCurve^>(pDef->Curve);
		if (tc != nullptr)
			bspline = dynamic_cast<IIfcBSplineCurveWithKnots^>(tc->BasisCurve);
		else //it might just be a bspline
			bspline = dynamic_cast<IIfcBSplineCurveWithKnots^>(pDef->Curve);
	}
	if (bspline == nullptr) return false; //no need to fix if its not an IfcBSplineCurveWithKnots
	if (surfaceOfLinearExtrusions->Position == nullptr) return false;

	if (!ShouldApplyRevitIncorrectBsplineSweptCurve()) return false;
	TopLoc_Location newLoc;
	if (GEOMETRY_FACTORY->ToLocation(surfaceOfLinearExtrusions->Position, newLoc))
		fixedEdge.Move(newLoc);
	return true;
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::ShouldApplyRevitIncorrectBsplineSweptCurve()
{
	if (!ApplyRevitIncorrectBsplineSweptCurve.HasValue) InitRevitWorkArounds();
	return ApplyRevitIncorrectBsplineSweptCurve.Value;
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::ShouldApplyRevitIncorrectArcCentreSweptCurve()
{
	if (!ApplyRevitIncorrectArcCentreSweptCurve.HasValue) InitRevitWorkArounds();
	return ApplyRevitIncorrectArcCentreSweptCurve.Value;
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::ShouldApplyPolylineTrimLengthOneForEntireLine()
{
	if (!ApplyPolylineTrimLengthOneForEntireLine.HasValue) InitRevitWorkArounds();
	return ApplyPolylineTrimLengthOneForEntireLine.Value;
	
}

bool Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::ShouldApplyRevitSweptSurfaceExtrusionInFeet()
{
	if (!ApplyRevitSweptSurfaceExtrusionInFeet.HasValue) InitRevitWorkArounds();
	return ApplyRevitSweptSurfaceExtrusionInFeet.Value;
}

void Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds::InitRevitWorkArounds()
{
	//always apply this
	ApplyPolylineTrimLengthOneForEntireLine = true;
	//it looks like all revit exports up to the 2020 release do not consider the local placement, so broadening the previous catch
	auto header = ModelGeometryService->Model->Header;

	//typical pattern for the revit exporter
	System::String^ revitPattern = "- Exporter (\\d*.\\d*.\\d*.\\d*)";
	System::String^ revitAltUIPattern = "- Exporter- Alternate UI (\\d*.\\d*.\\d*.\\d*)";
	if (header->FileName == nullptr || System::String::IsNullOrWhiteSpace(header->FileName->OriginatingSystem))
		return; //nothing to do
	auto matches = Regex::Matches(header->FileName->OriginatingSystem, revitPattern, RegexOptions::IgnoreCase);
	auto matchesAltUI = Regex::Matches(header->FileName->OriginatingSystem, revitAltUIPattern, RegexOptions::IgnoreCase);
	System::String^ version = nullptr;
	if (matches->Count > 0 && matches[0]->Groups->Count == 2)
		version = matches[0]->Groups[1]->Value;
	else if (matchesAltUI->Count > 0 && matchesAltUI[0]->Groups->Count == 2)
		version = matchesAltUI[0]->Groups[1]->Value;
	if (matches->Count > 0 || matchesAltUI->Count > 0) //looks like Revit
	{
		//SurfaceOfLinearExtrusion bug found in all current versions, comment this code out when it is fixed               
		ApplyRevitIncorrectArcCentreSweptCurve = true;
		ApplyRevitSweptSurfaceExtrusionInFeet = true;
		if (!System::String::IsNullOrEmpty(version)) //we have the build versions
		{
			System::Version^ modelVersion;
			if (System::Version::TryParse(version, modelVersion))
			{
				//uncomment this code when it is fixed in the exporter
				////SurfaceOfLinearExtrusion bug found in version 20.1.0 and earlier
				//var revitIncorrectArcCentreSweptCurveVersion = new Version(20, 1, 0, 0);
				//if (modelVersion <= revitIncorrectArcCentreSweptCurveVersion)
				//{
				//    modelFactors.AddWorkAround(RevitIncorrectArcCentreSweptCurve);
				//}
				//SurfaceOfLinearExtrusion bug found in version 20.0.0 and earlier
				auto revitIncorrectBsplineSweptCurveVersion = gcnew System::Version(20, 0, 0, 500);
				if (modelVersion <= revitIncorrectBsplineSweptCurveVersion)
					ApplyRevitIncorrectBsplineSweptCurve = true;

			}

		}
	}
	else
	{
		ApplyRevitIncorrectArcCentreSweptCurve = false;
		ApplyRevitSweptSurfaceExtrusionInFeet = false;
		ApplyRevitIncorrectBsplineSweptCurve = false;
		
	}
}
