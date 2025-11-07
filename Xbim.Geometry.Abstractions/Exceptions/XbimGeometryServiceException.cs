using System;

namespace Xbim.Geometry.Exceptions
{
    public class XbimGeometryServiceException : Exception
    {
        public XbimGeometryServiceException() { }
        public XbimGeometryServiceException(string message) : base(message) { }
        public XbimGeometryServiceException(string message, Exception innerException) : base(message, innerException) { }
    }
}
