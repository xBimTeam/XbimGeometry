#pragma once

namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryFactoryException :
                public System::Exception
            {
            public:
                XbimGeometryFactoryException() {};
                XbimGeometryFactoryException(System::String^ message) : System::Exception(message) { };
                XbimGeometryFactoryException(System::String^ message, System::Exception^ inner) : System::Exception(message, inner) {};
            };
        }
    }
}


