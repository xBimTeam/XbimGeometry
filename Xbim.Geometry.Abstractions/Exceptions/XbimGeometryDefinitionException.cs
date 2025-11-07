using System;

namespace Xbim.Geometry.Exceptions
{
    public class XbimGeometryDefinitionException : Exception
    {
        public XbimGeometryDefinitionException() { }
        public XbimGeometryDefinitionException(string message) : base(message) { }
        public XbimGeometryDefinitionException(string message, Exception innerException) : base(message, innerException) { }
    }
}
