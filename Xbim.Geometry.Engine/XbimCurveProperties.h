#pragma once
#include <Geom_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>

public ref class XbimCurveProperties
{
private:
	Handle(Geom_Curve) _curve;
public:

	XbimCurveProperties(Handle(Geom_Curve) curve)
	{
		_curve = curve;
	}

	virtual double Length{ double get() { GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(_curve)); } }
};

