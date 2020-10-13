#pragma once


using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref struct Vector3
		{
		public:
			Vector3::Vector3(System::Single x, System::Single y, System::Single z)
			{
				X = x;
				Y = y;
				Z = z;
			};
			System::Single X;
			System::Single Y;
			System::Single Z;
		};

		ref class XbimMesh
		{

		private:
			array<System::Byte>^ _array = nullptr;
		public:
			const int VersionPos = 0;
			const int VertexCountPos = VersionPos + sizeof(System::Byte);
			const int TriangleCountPos = VertexCountPos + sizeof(int);
			const int VertexPos = TriangleCountPos + sizeof(int);
			XbimMesh::XbimMesh(array<System::Byte>^ meshData) { _array = meshData; }
			virtual property System::Byte Version { System::Byte get() { return (_array != nullptr && _array->Length > 0) ? _array[VersionPos] : (System::Byte)0; } };
			virtual property int VertexCount {int get() {
				return (_array != nullptr && _array->Length > 0) ? System::BitConverter::ToInt32(_array, VertexCountPos) : 0;
			}};
			virtual property int TriangleCount { int get() { return (_array != nullptr && _array->Length > 0) ? System::BitConverter::ToInt32(_array, TriangleCountPos) : 0; }}
			virtual property int FaceCount {int get() {
				int faceCountPos = VertexPos + (VertexCount * 3 * sizeof(float));
				return (_array != nullptr && _array->Length > 0) ? System::BitConverter::ToInt32(_array, faceCountPos) : 0;
			}}
			virtual property  int Length {int get() {
				return (_array != nullptr && _array->Length > 0) ? _array->Length : 0;
			}}
			virtual property  array<System::Byte>^ ToByteArray { array<System::Byte>^ get() { return _array; }}


			List<Vector3^>^ Vertices()
			{
				const int offsetY = sizeof(float);
				const int offsetZ = 2 * sizeof(float);
				List<Vector3^>^ vertices = gcnew List<Vector3^>(VertexCount);
				for (int i = 0; i < VertexCount; i++)
				{
					int p = VertexPos + (i * 3 * sizeof(float));
					vertices->Add(gcnew Vector3(System::BitConverter::ToSingle(_array, p), System::BitConverter::ToSingle(_array, p + offsetY), System::BitConverter::ToSingle(_array, p + offsetZ)));
				}
				return vertices;
			}

		};

	}
}