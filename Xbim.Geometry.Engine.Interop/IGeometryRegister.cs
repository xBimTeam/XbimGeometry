using Xbim.Common;
using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.Engine.Interop
{
    public interface IGeometryRegistration
    {
        void RegisterModel(IModel model, XGeometryEngineVersion geometryEngineVersion = XGeometryEngineVersion.V6);

        void UnregisterModel(IModel model);
    }
}
