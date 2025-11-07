using System.Collections.Generic;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeFactory : IXModelScoped
    {
		IXShape Convert(string shape);
		//IXbimGeometryObject ConvertToV5(string brepStr);
		//IXbimGeometryObject ConvertToV5(IXShape shape);
		string Convert(IXShape shape);
		string Convert(IXbimGeometryObject shape);

		IXShape UnifyDomain(IXShape toFix);

		IXShape Build(IIfcGeometricRepresentationItem geomRep);

		IXShape Transform(IXShape shape, XbimMatrix3D matrix);
		IXShape Union(IXShape body, IXShape addition);
		IXShape Cut(IXShape body, IXShape substraction);
		IXShape Union(IXShape body, IEnumerable<IXShape> addition);
		IXShape Cut(IXShape body, IEnumerable<IXShape> substraction);

		IXShape RemovePlacement(IXShape shape);
		IXShape SetPlacement(IXShape shape, IIfcObjectPlacement placement);


		IXShape Moved(IXShape shape, IIfcObjectPlacement placement, bool invertPlacement);
		IXShape Moved(IXShape shape, IXLocation moveTo);
		IXFace Add(IXFace toFace, IXWire[] wires);

		IEnumerable<IXFace> FixFace(IXFace face);
		
		

    }
}
