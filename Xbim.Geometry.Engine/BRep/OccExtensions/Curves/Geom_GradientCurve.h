#pragma once
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Standard_Type.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Standard_NotImplemented.hxx>
#include <vector>
#include "./Geom_ConvertibleToBSpline.h"


class Geom_GradientCurve;
DEFINE_STANDARD_HANDLE(Geom_GradientCurve, Geom_ConvertibleToBSpline)

class Geom_GradientCurve : public Geom_ConvertibleToBSpline {
public:
    Geom_GradientCurve(
        const Handle(Geom2d_Curve)& horizontalCurve,
        const Handle(Geom2d_Curve)& heightFunction,
        const gp_Pnt& endPoint,
        const bool hasEndPoint)
        : _horizontalCurve(horizontalCurve),
        _heightFunction(heightFunction),
        _endPoint(endPoint),
        _hasEndPoint(hasEndPoint)
    {
        _firstParam = _horizontalCurve->FirstParameter();
        _lastParam = _horizontalCurve->LastParameter();

        if (_hasEndPoint)
        {
            Standard_Real endParam = ComputeParameterAtEndPoint();
            _lastParam = endParam;
        }
    }
     

    void D0(Standard_Real U, gp_Pnt& P) const
    {

        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt2d horizontalPnt;
        _horizontalCurve->D0(U, horizontalPnt);

        gp_Pnt2d heightPnt;
        _heightFunction->D0(U, heightPnt);

        P.SetCoord(horizontalPnt.X(), horizontalPnt.Y(), heightPnt.Y());
    }

    void D1(Standard_Real U, gp_Pnt& P, gp_Vec& V) const
    {
        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt2d horizontalPnt;
        gp_Vec2d horizontalVec;
        _horizontalCurve->D1(U, horizontalPnt, horizontalVec);

        gp_Pnt2d heightPnt;
        gp_Vec2d heightVec;
        _heightFunction->D1(U, heightPnt, heightVec);

        P.SetCoord(horizontalPnt.X(), horizontalPnt.Y(), heightPnt.Y());
        V.SetCoord(horizontalVec.X(), horizontalVec.Y(), heightVec.Y());
    }


    void D2(Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const
    {
        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt2d horizontalPnt;
        gp_Vec2d horizontalVec1, horizontalVec2;
        _horizontalCurve->D2(U, horizontalPnt, horizontalVec1, horizontalVec2);

        gp_Pnt2d heightPnt;
        gp_Vec2d heightVec1, heightVec2;
        _heightFunction->D2(U, heightPnt, heightVec1, heightVec2);

        P.SetCoord(horizontalPnt.X(), horizontalPnt.Y(), heightPnt.Y());
        V1.SetCoord(horizontalVec1.X(), horizontalVec1.Y(), heightVec1.Y());
        V2.SetCoord(horizontalVec2.X(), horizontalVec2.Y(), heightVec2.Y());
    }

    void D3(Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const
    {
        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt2d horizontalPnt;
        gp_Vec2d horizontalVec1, horizontalVec2, horizontalVec3;
        _horizontalCurve->D3(U, horizontalPnt, horizontalVec1, horizontalVec2, horizontalVec3);

        gp_Pnt2d heightPnt;
        gp_Vec2d heightVec1, heightVec2, heightVec3;
        _heightFunction->D3(U, heightPnt, heightVec1, heightVec2, heightVec3);

        P.SetCoord(horizontalPnt.X(), horizontalPnt.Y(), heightPnt.Y());
        V1.SetCoord(horizontalVec1.X(), horizontalVec1.Y(), heightVec1.Y());
        V2.SetCoord(horizontalVec2.X(), horizontalVec2.Y(), heightVec2.Y());
        V3.SetCoord(horizontalVec3.X(), horizontalVec3.Y(), heightVec3.Y());
    }


    virtual gp_Vec DN(Standard_Real U, Standard_Integer N) const {
        if (N == 1) {
            gp_Pnt P;
            gp_Vec V1;
            D1(U, P, V1);
            return V1;
        }
        else if (N == 2) {
            gp_Pnt P;
            gp_Vec V1, V2;
            D2(U, P, V1, V2);
            return V2;
        }
        else if (N == 3) {
            gp_Pnt P;
            gp_Vec V1, V2, V3;
            D3(U, P, V1, V2, V3);
            return V2;
        }
        else {
            Standard_NotImplemented::Raise("DN not implemented for N > 3");
            return gp_Vec();
        }
    }

    Standard_Real FirstParameter() const
    {
        return _firstParam;
    }

    Standard_Real LastParameter() const
    {
        return _lastParam;
    }

    GeomAbs_Shape Continuity() const
    {
        return _horizontalCurve->Continuity();
    }

    Standard_Boolean IsClosed() const
    {
        gp_Pnt PStart, PEnd;
        D0(FirstParameter(), PStart);
        D0(LastParameter(), PEnd);

        return PStart.IsEqual(PEnd, Precision::Confusion());
    }

    Standard_Boolean IsPeriodic() const
    {
        return _horizontalCurve->IsPeriodic();
    }

    void Reverse()
    {
        _horizontalCurve->Reverse();
        _heightFunction->Reverse();

        // Adjust the parameter range
        Standard_Real temp = _firstParam;
        _firstParam = -_lastParam;
        _lastParam = -temp;
    }

    Standard_Real ReversedParameter(Standard_Real U) const
    {
        return _firstParam + _lastParam - U;
    }

    void Transform(const gp_Trsf& T)
    {
        Standard_NotImplemented::Raise("Transform not implemented");
    }

    Handle(Geom_Geometry) Copy() const
    {
        Handle(Geom2d_Curve) horizontalCopy = Handle(Geom2d_Curve)::DownCast(_horizontalCurve->Copy());
        Handle(Geom2d_Curve) heightCopy = Handle(Geom2d_Curve)::DownCast(_heightFunction->Copy());

        return new Geom_GradientCurve(horizontalCopy, heightCopy, _endPoint, _hasEndPoint);
    }

    Standard_Boolean IsCN(Standard_Integer N) const {
        return _horizontalCurve->IsCN(N);
    }

    Handle(Geom_BSplineCurve) ToBSpline(int nbPoints) const override
    {
        TColgp_Array1OfPnt points(1, nbPoints + (_hasEndPoint ? 1 : 0));
        GetPointsFromProjectionAndHeightCurves(points, nbPoints);
        if (_hasEndPoint) {
            points.SetValue(nbPoints + 1, _endPoint);
        }
        GeomAPI_PointsToBSpline pointsToBSpline(points, 8, 8, GeomAbs_CN);
        return pointsToBSpline.Curve();
    }


    Standard_Real ComputeParameterAtEndPoint()
    {
        auto func = [this](Standard_Real U) -> Standard_Real {
            gp_Pnt P;
            this->D0(U, P);
            return P.Distance(_endPoint);
            };

        Standard_Real tol = 1e-6;
        Standard_Real low = _firstParam;
        Standard_Real high = _lastParam;

        while (high - low > tol) {
            Standard_Real mid = (low + high) / 2.0;
            Standard_Real distMid = func(mid);
            Standard_Real distLow = func(low);
            Standard_Real distHigh = func(high);

            if (distMid < tol) {
                return mid;
            }

            if (distLow < distHigh) {
                high = mid;
            }
            else {
                low = mid;
            }
        }

        return (low + high) / 2.0;
    }



    TColgp_Array1OfPnt GetPointsFromProjectionAndHeightCurves(TColgp_Array1OfPnt& points, Standard_Integer nbPoints)const {

        double projectionParm1 = _horizontalCurve->FirstParameter();
        double projectionParm2 = _horizontalCurve->LastParameter();

        Geom2dAdaptor_Curve adaptorHorizontalProjection(_horizontalCurve, projectionParm1, projectionParm2);
        GCPnts_UniformAbscissa uniformAbscissa(adaptorHorizontalProjection, nbPoints);

        for (Standard_Integer i = 1; i <= nbPoints; ++i) {

            Standard_Real param = uniformAbscissa.Parameter(i);
            gp_Pnt2d p2d;
            _horizontalCurve->D0(param, p2d);
            gp_Pnt2d heightPoint;
            _heightFunction->D0(param, heightPoint);

            // For the Height Function:
            // Y coord of the curve is the Z ccord of the 3D point
            Standard_Real z = heightPoint.Y();
            gp_Pnt p3d(p2d.X(), p2d.Y(), z);
            points.SetValue(i, p3d);
        }
        return points;
    }


    const Handle(Geom2d_Curve)& HorizontalCurve() {
        return _horizontalCurve;
    }

    const Handle(Geom2d_Curve)& HeightFunction() {
        return _heightFunction;
    }

private:
    Handle(Geom2d_Curve) _horizontalCurve;
    Handle(Geom2d_Curve) _heightFunction;
    Standard_Real _firstParam;
    Standard_Real _lastParam;
    gp_Pnt _endPoint;
    bool _hasEndPoint;
};

