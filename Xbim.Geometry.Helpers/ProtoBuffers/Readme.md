These are the protocol buffers for geometry and wexbim

to compile the source in the current directory

protoc-3.4.0-windows-x86_64.exe -I=./ --csharp_out=./ ./Geometry.proto

protoc-3.4.0-windows-x86_64.exe -I=./ --csharp_out=./ ./WexBim.proto

For further help see

https://developers.google.com/protocol-buffers/

https://github.com/google/protobuf
