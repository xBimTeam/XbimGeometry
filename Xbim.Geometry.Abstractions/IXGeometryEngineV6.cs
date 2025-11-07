using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXGeometryEngineV6 : IXbimGeometryEngine, IXModelGeometryService
    {
        IXModelGeometryService ModelGeometryService { get; }
        IXShape Build(IIfcGeometricRepresentationItem geomRep);
    }
}
