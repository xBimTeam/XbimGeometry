using System;

namespace Xbim.Geometry.Exceptions
{
    public class XbimGeometryFactoryException : Exception
    {
        public XbimGeometryFactoryException() { }
        public XbimGeometryFactoryException(string message) : base(message) { }
        public XbimGeometryFactoryException(string message, Exception innerException) : base(message, innerException) { }
    }
}
