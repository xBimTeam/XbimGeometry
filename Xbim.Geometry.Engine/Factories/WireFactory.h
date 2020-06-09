#pragma once
#include "../XbimHandle.h"
#include "../BRep/XbimWire.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include "GeomProcFactory.h"
#include "./Unmanaged/NWireFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "CurveFactory.h"
#include <BRep_Builder.hxx>
#include <Geom_Surface.hxx>
#include <TColGeom_SequenceOfCurve.hxx>

using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4::MeasureResource;
using namespace Xbim::Ifc4::ProfileResource;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Threading::Tasks;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class WireFactory : XbimHandle<NWireFactory>
			{
			private:
				IXLoggingService^ LoggerService;
				IXModelService^ ModelService;
				//The distance between two points at which they are determined to be equal points

				GeomProcFactory^ GPFactory;
				CurveFactory^ _curveFactory;

				TopoDS_Wire Build3d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dCircle(IIfcCircle^ ifcCircle, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dTrimmedCurve(IIfcTrimmedCurve^ ifcTrimmedCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dPolyline(IIfcPolyline^ ifcPolyline, Handle(Geom_Surface)& surface);
				//builds a trimmed 3d polyline param values of -1 are taken as no trim
				TopoDS_Wire Build3D(IIfcPolyline^ ifcPolyline, double startParam, double endParam);
				void BuildSegments(IIfcCompositeCurve^ ifcCurve, TColGeom_SequenceOfCurve& segments, bool sameSense);
				void BuildSegments(IIfcCompositeCurve^ ifcCurve, TColGeom_SequenceOfCurve& segments) { BuildSegments(ifcCurve, segments, true); };
				TopoDS_Wire MakeWire(Handle(Geom_Curve) curve);
				template<typename IfcType>
				Handle(Geom_Curve)  BuildCompositeCurveSegment(IfcType ifcCurve, bool sameSense, bool isTrimmedCurve);
				template<typename IfcType>
				Handle(Geom_Curve)  BuildCompositeCurveSegment(IfcType ifcCurve, bool sameSense) { return BuildCompositeCurveSegment(ifcCurve, sameSense, false); };
				TopoDS_Wire BuildDirectrix(IIfcLine^ curve, double startParam, double endParam);
				TopoDS_Wire BuildDirectrix(IIfcCircle^ curve, double startParam, double endParam);
				TopoDS_Wire BuildDirectrix(IIfcEllipse^ curve, double startParam, double endParam);
				TopoDS_Wire BuildDirectrix(IIfcTrimmedCurve^ curve, double startParam, double endParam);
				TopoDS_Wire BuildDirectrix(IIfcPolyline^ curve, double startParam, double endParam);
				TopoDS_Wire BuildDirectrix(IIfcCompositeCurve^ curve, double startParam, double endParam);


				TopoDS_Wire BuildProfileDef(IIfcRectangleProfileDef^ profile);

			internal:
				TopoDS_Wire BuildDirectrix(IIfcCurve^ curve, Nullable<IfcParameterValue> startParam, Nullable<IfcParameterValue> endParam);
				TopoDS_Wire BuildProfile(IIfcProfileDef^ profileDef);
				//Builds an IfcCurve as a TopoDS_Wire
				TopoDS_Wire BuildWire(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);

				/*void GetCurves(IIfcPolyline^ polyline, TColGeom_SequenceOfCurve& curves);
				void GetCurves(IIfcCompositeCurve^ compCurve, TColGeom_SequenceOfCurve& curves);
				void GetCurves(IIfcCompositeCurveOnSurface^ compCurve, TColGeom_SequenceOfCurve& curves);
				void GetCurves(IIfcIndexedPolyCurve^ compCurve, TColGeom_SequenceOfCurve& curves);
				void GetCurves(IIfcCurve^ curve, TColGeom_SequenceOfCurve& curves);*/
			public:
				WireFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NWireFactory())
				{
					LoggerService = loggingService;
					ModelService = modelService;
					GPFactory = gcnew GeomProcFactory(loggingService, modelService);
					_curveFactory = gcnew CurveFactory(loggingService, modelService);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}


				virtual IXWire^ Build(IIfcCurve^ ifcCurve);
				virtual IXWire^ Build(IIfcProfileDef^ ifcProfileDef);

			};

		}
	}
}

