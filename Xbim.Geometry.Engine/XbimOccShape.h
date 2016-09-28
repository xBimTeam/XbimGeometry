#pragma once
#include "XbimGeometryObject.h"
#include <TopoDS_Shape.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <OSD_Timer.hxx>
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
#ifndef XBIMPROGRESSINDICATOR_H
#define XBIMPROGRESSINDICATOR_H

# include <Standard_DefineHandle.hxx>
# include <Standard_Macro.hxx>
# include <Message_ProgressIndicator.hxx>

//DEFINE_STANDARD_HANDLE(XbimProgressIndicator, Message_ProgressIndicator)
class XbimProgressIndicator : public Message_ProgressIndicator
{
private:
	OSD_Timer aTimer;
	Standard_Real maxRunDuration;
	bool timedOut;
public:
	XbimProgressIndicator(Standard_Real maxDurationSeconds, bool startTimer=true);
	virtual Standard_Boolean Show(const Standard_Boolean force) { return true; }
	virtual Standard_Boolean UserBreak();
	void StartTimer() { timedOut = false;  aTimer.Start(); }
	void StopTimer() { aTimer.Stop(); }
	Standard_Real ElapsedTime() { return aTimer.ElapsedTime(); }
	bool TimedOut() { return timedOut; }
	/*DEFINE_STANDARD_RTTI(XbimProgressIndicator, Message_ProgressIndicator)*/
};
#endif

namespace Xbim
{
	namespace Geometry
	{
		

		ref class XbimOccShape abstract : XbimGeometryObject
		{
		
		
			
		public:
			static void WriteIndex(BinaryWriter^ bw, UInt32 index, UInt32 maxInt);
			XbimOccShape();
			//operators
			virtual operator const TopoDS_Shape& () abstract;
			void WriteTriangulation(TextWriter^ textWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(IXbimMeshReceiver^ mesh, double tolerance, double deflection, double angle);
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual XbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator ^transformation) abstract;
			virtual XbimGeometryObject^ Moved(IIfcPlacement ^placement) abstract;
			virtual XbimGeometryObject^ Moved(IIfcObjectPlacement ^objectPlacement) abstract;
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle)
			{
				WriteTriangulation(mesh, precision, deflection, angle);
			}
			// Inherited via XbimGeometryObject

			
			
		};
	}
}

