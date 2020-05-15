#pragma once
using namespace System;
namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryDefinitionException :
                public Exception
            {
            public:
                XbimGeometryDefinitionException() {};
                XbimGeometryDefinitionException(String^ message) : Exception(message) { };
                XbimGeometryDefinitionException(String^ message, Exception^ inner) : Exception(message, inner) {};
            };
        }
    }
}


