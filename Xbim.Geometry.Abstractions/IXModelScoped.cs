namespace Xbim.Geometry.Abstractions
{
    public interface IXModelScoped
    {      
        IXModelGeometryService ModelGeometryService { get; }
        IXLoggingService LoggingService { get; }
    }
}
