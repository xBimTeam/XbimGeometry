using System;
using System.Collections.Generic;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Abstractions.WexBim;

namespace Xbim.Geometry.WexBim
{


    internal delegate int ReadIndex(byte[] array, int offset);

    /// <summary>
    /// Class representing an xbim Mesh in the wexbim format
    /// </summary>
    public class WexBimMesh : IWexBimMesh
    {
        private byte[] _array;
        const int VersionPos = 0;
        const int VertexCountPos = VersionPos + sizeof(byte);
        const int TriangleCountPos = VertexCountPos + sizeof(int);
        const int VertexPos = TriangleCountPos + sizeof(int);

        /// <summary>
        /// Constructs a new <see cref="WexBimMesh"/> from a <see cref="byte"/>[]
        /// </summary>
        /// <param name="meshData"></param>
        public WexBimMesh(byte[] meshData)
        {
            _array = meshData;
        }

        /// <summary>
        /// Gets the version of the wexbim file
        /// </summary>
        public byte Version => _array?.Length > 0 ? _array[VersionPos] : (byte)0;

        /// <summary>
        /// Gets the number of Vertices in the file
        /// </summary>
        public int VertexCount => _array?.Length > 0 ? BitConverter.ToInt32(_array, VertexCountPos) : 0;

        /// <summary>
        /// Gets the number of Triangles in the file
        /// </summary>
        public int TriangleCount => _array?.Length > 0 ? BitConverter.ToInt32(_array, TriangleCountPos) : 0;

        /// <summary>
        /// Gets the number of Faces in the file
        /// </summary>
        public int FaceCount
        {
            get
            {
                var faceCountPos = VertexPos + (VertexCount * 3 * sizeof(float));
                return _array?.Length > 0 ? BitConverter.ToInt32(_array, faceCountPos) : 0;
            }
        }

        /// <summary>
        /// Gets the size of the file in bytes
        /// </summary>
        public int Length => _array?.Length > 0 ? _array.Length : 0;

        /// <summary>
        /// Gets the underlying <see cref="byte"/>[] of the file
        /// </summary>
        /// <returns></returns>
        public byte[] ToByteArray() => _array;


        /// <summary>
        /// Returns the Vertices
        /// </summary>
        public IEnumerable<IFloat3> Vertices
        {
            get
            {
                const int offsetY = sizeof(float);
                const int offsetZ = 2 * sizeof(float);
                for (int i = 0; i < VertexCount; i++)
                {
                    var p = VertexPos + (i * 3 * sizeof(float));
                    yield return new Vector3(BitConverter.ToSingle(_array, p), BitConverter.ToSingle(_array, p + offsetY), BitConverter.ToSingle(_array, p + offsetZ));
                }
            }
        }
        /// <summary>
        /// Returns the vector at the specified position
        /// </summary>
        /// <param name="vectorIndex"></param>
        /// <returns></returns>
        public Vector3 this[int vectorIndex]
        {
            get
            {
                const int offsetY = sizeof(float);
                const int offsetZ = 2 * sizeof(float);

                var p = VertexPos + (vectorIndex * 3 * sizeof(float));
                return new Vector3(BitConverter.ToSingle(_array, p), BitConverter.ToSingle(_array, p + offsetY), BitConverter.ToSingle(_array, p + offsetZ));

            }
        }

        /// <summary>
        /// Gets the Faces
        /// </summary>
        public IEnumerable<IWexBimMeshFace> Faces
        {
            get
            {
                var faceOffset = VertexPos + (VertexCount * 3 * sizeof(float)) + sizeof(int);//start of vertices * space taken by vertices + the number of faces
                ReadIndex readIndex;
                int sizeofIndex;
                if (VertexCount <= 0xFF)
                {
                    readIndex = (array, offset) => array[offset];
                    sizeofIndex = sizeof(byte);
                }
                else if (VertexCount <= 0xFFFF)
                {
                    readIndex = (array, offset) => BitConverter.ToUInt16(array, offset);
                    sizeofIndex = sizeof(ushort);
                }
                else
                {
                    readIndex = (array, offset) => BitConverter.ToInt32(array, offset);
                    sizeofIndex = sizeof(int);
                }
                for (int i = 0; i < FaceCount; i++)
                {
                    var face = new WexBimMeshFace(readIndex, sizeofIndex, _array, faceOffset);
                    faceOffset += face.ByteSize;
                    yield return face;
                }
            }
        }

        

    }

    /// <summary>
    /// Class representing a wexbim face mesh
    /// </summary>
    internal class WexBimMeshFace : IWexBimMeshFace
    {
        private byte[] _array;
        private int _offsetStart;
        private ReadIndex _readIndex;
        private int _sizeofIndex;

        internal WexBimMeshFace(ReadIndex readIndex, int sizeofIndex, byte[] array, int faceOffset)
        {
            _readIndex = readIndex;
            _array = array;
            _offsetStart = faceOffset;
            _sizeofIndex = sizeofIndex;
        }
        public int ByteSize
        {
            get
            {
                if (IsPlanar)
                    return sizeof(int) + 2 + (TriangleCount * 3 * _sizeofIndex); // trianglecount+ normal in 2 bytes + triangulation with no normals
                else
                    return sizeof(int) + (TriangleCount * 3 * (_sizeofIndex + 2)); //trianglecount+ normal  + triangulation with normals in 2 bytes
            }
        }
        public int OffsetStart => _offsetStart;
        public int TriangleCount => Math.Abs(BitConverter.ToInt32(_array, _offsetStart));
        public bool IsPlanar => BitConverter.ToInt32(_array, _offsetStart) > 0;
        public IEnumerable<int> Indices
        {
            get
            {

                if (IsPlanar)
                {
                    var indexOffset = _offsetStart + sizeof(int) + 2; //offset + trianglecount + packed normal of plane in the 2 bytes
                    var indexSpan = 3 * _sizeofIndex;
                    for (int i = 0; i < TriangleCount; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            yield return _readIndex(_array, indexOffset + (j * _sizeofIndex)); //skip the normal in the 2 bytes
                        }
                        indexOffset += indexSpan;
                    }
                }
                else
                {
                    var indexOffset = _offsetStart + sizeof(int); //offset + trianglecount
                    var indexSpan = _sizeofIndex + 2; //index  + normal in 2 bytes
                    var triangleSpan = 3 * indexSpan;

                    for (int i = 0; i < TriangleCount; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            yield return _readIndex(_array, indexOffset + (j * indexSpan));
                        }
                        indexOffset += triangleSpan;
                    }
                }
            }
        }

        /// <summary>
        /// returns the normal for a specific point at a specific index on the face
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public Vector3 NormalAt(int index)
        {
            var indexOffset = _offsetStart + sizeof(int); //offset + trianglecount
            if (IsPlanar) //no matter what you send in for the index you will get the same value because it is planar
            {
                var u = _array[indexOffset];
                var v = _array[indexOffset + 1];
                return new Vector3(u, v);
            }
            else
            {
                var indexSpan = _sizeofIndex + 2;
                int normalOffset = indexOffset + (index * indexSpan) + _sizeofIndex;
                var u = _array[normalOffset];
                var v = _array[normalOffset + 1];
                return new Vector3(u, v);
            }
        }

        public byte[] ToByteArray()
        {
            throw new NotImplementedException();
        }

        IXDirection IWexBimMeshFace.NormalAt(int index)
        {
            throw new NotImplementedException();
        }

        public IEnumerable<IFloat3> Normals
        {
            get
            {
                var indexOffset = _offsetStart + sizeof(int); //offset + trianglecount
                if (IsPlanar)
                {
                    var u = _array[indexOffset];
                    var v = _array[indexOffset + 1];

                    yield return new Vector3(u, v);
                }
                else
                {
                    var indexSpan = _sizeofIndex + 2;
                    var triangleSpan = 3 * indexSpan;
                    var normalOffset = indexOffset + _sizeofIndex;
                    for (int i = 0; i < TriangleCount; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            var u = _array[normalOffset + (j * indexSpan)];
                            var v = _array[normalOffset + (j * indexSpan) + 1];
                            yield return new Vector3(u, v);

                        }
                        normalOffset += triangleSpan;
                    }
                }
            }
        }

        
    }

    /// <summary>
    /// A three dimensional vector
    /// </summary>
    public struct Vector3 : IFloat3
    {
        const double PackSize = 252;
        const double PackTolerance = 1e-4;
        float _x; float _y; float _z;

        /// <summary>
        /// The X component of the vector
        /// </summary>
        public float X => _x;
        /// <summary>
        /// The Y component of the vector
        /// </summary>

        public float Y => _y;
        /// <summary>
        /// The Z component of the vector
        /// </summary>

        public float Z => _z;


        /// <summary>
        /// Constructs a new vector from UV coordinates
        /// </summary>
        /// <param name="u"></param>
        /// <param name="v"></param>
        public Vector3(short u, short v) : this()
        {
            var lon = u / PackSize * Math.PI * 2;
            var lat = v / PackSize * Math.PI;

            _y = (float)Math.Cos(lat);
            _x = (float)((Math.Sin(lon) * Math.Sin(lat)));
            _z = (float)((Math.Cos(lon) * Math.Sin(lat)));

        }

        /// <summary>
        /// Constructs a new vector from X, Y and Z components
        /// </summary>
        /// <param name="v1"></param>
        /// <param name="v2"></param>
        /// <param name="v3"></param>
        public Vector3(float v1, float v2, float v3)
        {
            _x = v1;
            _y = v2;
            _z = v3;

        }
    }
}
