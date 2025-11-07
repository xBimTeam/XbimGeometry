using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSolidService : IXModelScoped
    {

        IXShape Union(IXShape body, IXShape addition, double precision);
        IXShape Cut(IXShape body, IXShape subtraction, double precision);
        IXShape Intersect(IXShape body, IXShape other, double precision);
        IXShape Union(IXShape body, IEnumerable<IXShape> additions, double precision);
        IXShape Cut(IXShape body, IEnumerable<IXShape> subtractions, double precision);
        IXShape Intersect(IXShape body, IEnumerable<IXShape> others, double precision);
        IEnumerable<IXSolid> CollectSolids(IXShape collectFrom);
        IXShape Build(IIfcGeometricRepresentationItem geomRep);
        Task<IXShape> BuildAsync(IIfcGeometricRepresentationItem geomRep);
        Task<IXShape> BuildAsync(IIfcGeometricRepresentationItem geomRep, CancellationToken token);


        IXShape Build(IIfcSolidModel ifcSolid);
        Task<IXShape> BuildAsync(IIfcSolidModel ifcSolid);
        Task<IXShape> BuildAsync(IIfcSolidModel ifcSolid, CancellationToken token);


        IXSolid Build(IIfcCsgPrimitive3D ifcCsgPrimitive);
        Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive);
        Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive, CancellationToken token);


      

       
    }
}
