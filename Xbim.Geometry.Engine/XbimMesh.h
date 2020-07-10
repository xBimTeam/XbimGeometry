#pragma once

using namespace System;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref struct Vector3
		{
		public:
			Vector3::Vector3(Single x, Single y, Single z)
			{
				X = x;
				Y = y;
				Z = z;
			};
			Single X;
			Single Y;
			Single Z;
		};

		ref class XbimMesh
		{

		private:
			array<Byte>^ _array = nullptr;
		public:
			const int VersionPos = 0;
			const int VertexCountPos = VersionPos + sizeof(Byte);
			const int TriangleCountPos = VertexCountPos + sizeof(int);
			const int VertexPos = TriangleCountPos + sizeof(int);
			XbimMesh::XbimMesh(array<Byte>^ meshData) { _array = meshData; }
			virtual property Byte Version { Byte get() { return (_array != nullptr && _array->Length > 0) ? _array[VersionPos] : (Byte)0; } };
			virtual property int VertexCount {int get() {
				return (_array != nullptr && _array->Length > 0) ? BitConverter::ToInt32(_array, VertexCountPos) : 0;
			}};
			virtual property int TriangleCount { int get() { return (_array != nullptr && _array->Length > 0) ? BitConverter::ToInt32(_array, TriangleCountPos) : 0; }}
			virtual property int FaceCount {int get() {
				int faceCountPos = VertexPos + (VertexCount * 3 * sizeof(float));
				return (_array != nullptr && _array->Length > 0) ? BitConverter::ToInt32(_array, faceCountPos) : 0;
			}}
			virtual property  int Length {int get() {
				return (_array != nullptr && _array->Length > 0) ? _array->Length : 0;
			}}
			virtual property  array<Byte>^ ToByteArray { array<Byte>^ get() { return _array; }}


			List<Vector3^>^ Vertices()
			{
				const int offsetY = sizeof(float);
				const int offsetZ = 2 * sizeof(float);
				List<Vector3^>^ vertices = gcnew List<Vector3^>(VertexCount);
				for (int i = 0; i < VertexCount; i++)
				{
					int p = VertexPos + (i * 3 * sizeof(float));
					vertices->Add(gcnew Vector3(BitConverter::ToSingle(_array, p), BitConverter::ToSingle(_array, p + offsetY), BitConverter::ToSingle(_array, p + offsetZ)));
				}
				return vertices;
			}

		};

	}
}