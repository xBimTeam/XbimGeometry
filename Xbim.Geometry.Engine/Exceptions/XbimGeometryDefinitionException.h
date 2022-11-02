#pragma once

namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryDefinitionException :
                public System::Exception
            {
            public:
                XbimGeometryDefinitionException() {};
                XbimGeometryDefinitionException(System::String^ message) : System::Exception(message) { };
                XbimGeometryDefinitionException(System::String^ message, System::Exception^ inner) : System::Exception(message, inner) {};
            };
        }
    }
}


