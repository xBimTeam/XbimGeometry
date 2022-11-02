#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"

#include <TopoDS_Shell.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <vector>


#include <TopoDS_Solid.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_ListOfShape.hxx>
#include <IMeshTools_Parameters.hxx>
#include "../../Services/Unmanaged/NLoggingService.h"
#include <ShapeExtend_Status.hxx>

class NShapeFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Shape _emptyShape;
	double _timeout;
	
public:
	NShapeFactory(double timout) : _timeout(timout)
	{
			
	}
	static bool Triangulate(const TopoDS_Shape& aShape, const IMeshTools_Parameters& meshParams);
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Shape UnifyDomain(const TopoDS_Shape& toFix, double linearTolerance, double angularTolerance);
	TopoDS_Solid UnifyDomain(const TopoDS_Solid& toFix, double linearTolerance, double angularTolerance);
	TopoDS_Shape Convert(const char* brepString);
	const char* Convert(TopoDS_Shape shape);

	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double minumGap);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double minumGap);
	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopTools_ListOfShape& subtractions, double minumGap);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopTools_ListOfShape& additions, double minumGap);
	ShapeExtend_Status FixFace(const TopoDS_Face& face, TopTools_ListOfShape& result);
	
};
