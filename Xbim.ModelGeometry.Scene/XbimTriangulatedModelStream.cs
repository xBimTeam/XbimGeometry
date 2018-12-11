/* ==================================================================

The stream has an unusual doble indirection to points and normals to be able to retain unique position idenity for those
frameworks that do not consider or require normal specifications (therefore saving streaming size and reducing video 
memory usage)

Structure of stream for triangular meshes:
CountUniquePositions		// int
CountUniqueNormals			// int
CountUniquePositionNormals	// int
CountAllTriangles			// int // used to prepare index array if needed
CountPolygons				// int
[PosX, PosY, PosZ]			// 3 * floats * CountUniquePositions
...
[NrmX, NrmY, NrmZ]			// 3 * floats * CountUniqueNormals
...
[iPos]						// int, short or byte f(CountUniquePositions)
...					
[iNrm]						// int, short or byte f(CountUniqueNormals)
...
[Polygons:  
	PolyType // byte
	PolygonLen // int
	[UniquePositionNormal]  // int, short or byte f(CountUniquePositions)
	...
]...


Example for a 1x1x1 box:

		8 // number of points
		6 // number of normals (one for each face)
		24 // each of the 8 points in a box belongs to 3 faces; it has therefore 3 normals
		12 // 2 triangles per face
		1  // all shape in one call
	+->	0, 0, 0 (index: 0)  // these are the 8 points
	|	0, 1, 0
	|	1, 0, 0
	|	1, 1, 0
	|	0, 0, 1
	|	0, 1, 1
	|	1, 0, 1
	|	1, 1, 1
	|	0, 0, 1 // top face normal
	|	1, 0, 0 // other normals...
	|	0, 1, 0
	|	0, 0, -1 
	|	-1, 0, 0 
+-> |	0, -1, 0 (index: 5)
|	|	// points indices (1 byte because of size)
|	|	4 (0) // first two triangles (unique point 0 to 3) point to unique positions 0,1,2,3
|	|	5 (1)
|	|	7 (2)
|	|	6 (3)
|	+-=	0 (4) <-------------------------------------------------------------------------+
|		[... omissis...]                                                                |
|		// normal indices                                                               |
|		0 (index:0) // first two triangles (unique point 0 to 3) share normal index 0   |
|		0 (index:1)                                                                     |
|		0 (index:2)                                                                     |
|		0 (index:3)                                                                     |
+---=	5 (index:4) <-------------------------------------------------------------------+
		5                                                                               |
		5                                                                               |
		[... omissis...]                                                                |
		// unique indices per polygon                                                   |
		//                                                                              |
		4 // polygons type (series of triangles)                                        |
		36 // lenght of the stream of indices for the first polygon                     |
		0 // first triangle of top face                                                 |
		1                                                                               |
		2                                                                               |
		0 // second of top face                                                         |
		2                                                                               |
		3                                                                               |
		4 // first triangle of front face is unique point 4 =---------------------------+ (pointing to position 0 and normal 5)
		5
		6
		[...more triangles follow...]
*/

using System.Linq;
using System.IO;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
	public enum TriangleType : byte
	{
		GL_Triangles = 0x0004,
        GL_Triangles_Strip = 0x0005,
        GL_Triangles_Fan = 0x0006
	}

	/// <summary>
	/// Binary stream encoded triangulated mesh; capable of builing other 
	/// </summary>
	public class XbimTriangulatedModelStream
	{
		public static readonly XbimTriangulatedModelStream Empty;

		static XbimTriangulatedModelStream()
		{
			Empty = new XbimTriangulatedModelStream(true);    
		}

		internal XbimTriangulatedModelStream(bool empty)
		{

		}
		public XbimTriangulatedModelStream()
		{
			_dataStream = new MemoryStream(0x4000);
		}
		public bool IsEmpty
		{
			get
			{
				return (this == Empty || _dataStream == null || _dataStream.Length == 0);
			}
		}

		MemoryStream _dataStream;

		public MemoryStream DataStream
		{
			get { return _dataStream; }
			set { _dataStream = value; }
		}

		public XbimTriangulatedModelStream(byte []  data)
		{
			_dataStream = new MemoryStream(0x4000);
			_dataStream.Write(data, 0, data.Length);
		}



		// writes the data to the xbimGC cache stream 
		//
		public void Write(BinaryWriter bw)
		{
			if (_dataStream != null)
			{
				// Debug.WriteLine(string.Format("Writing stream at: {0} len {1}.", bw.BaseStream.Position, _dataStream.Length));
				bw.Write((int)(_dataStream.Length));
				bw.Write(_dataStream.GetBuffer(), 0, (int)_dataStream.Length);
			}
			else
				// Debug.WriteLine(string.Format("Writing stream at: {0} empty.", bw.BaseStream.Position));
				bw.Write((int)0);
		}

		// this function seems only to be called in meshing XbimGeometryModelCollection
		// 
		public void MergeStream(XbimTriangulatedModelStream other)
		{
			if (other.IsEmpty)
				return;
			if (this.IsEmpty)
			{
				// just take the other stream
				_dataStream = other.DataStream;
				return;
			}

			BinaryReader[] r = new BinaryReader[] {
				new BinaryReader(this.DataStream), 
				new BinaryReader(other.DataStream)
			};

			MemoryStream result = new MemoryStream(0x4000);
			BinaryWriter w = new BinaryWriter(result);

			r[0].BaseStream.Seek(0, SeekOrigin.Begin);
			r[1].BaseStream.Seek(0, SeekOrigin.Begin);

			int[] CountUniquePositions = new int[2] { r[0].ReadInt32(), r[1].ReadInt32() };
			int[] CountUniqueNormals = new int[2] { r[0].ReadInt32(), r[1].ReadInt32() };
			int[] CountUniquePositionNormals = new int[2] { r[0].ReadInt32(), r[1].ReadInt32() };
			int[] CountAllTriangles = new int[2] { r[0].ReadInt32(), r[1].ReadInt32() };
			int[] CountPolygons = new int[2] { r[0].ReadInt32(), r[1].ReadInt32() };

			w.Write((uint)CountUniquePositions.Sum());
			w.Write((uint)CountUniqueNormals.Sum());
			w.Write((uint)CountUniquePositionNormals.Sum());
			w.Write((uint)CountAllTriangles.Sum());
			w.Write((uint)CountPolygons.Sum());


			// copies point coordinates
			for (int i = 0; i < 2; i++)
			{
				w.Write(r[i].ReadBytes(sizeof(float) * 3 * CountUniquePositions[i]));
			}
			// copies normal vectors
			for (int i = 0; i < 2; i++)
			{
				w.Write(r[i].ReadBytes(sizeof(float) * 3 * CountUniqueNormals[i]));
			}

			// indices to points need to be copied in consideration of remapping 
			uint offset = 0;
			uint sumout = (uint)CountUniquePositions.Sum();
			for (int i = 0; i < 2; i++)
			{
				IndexConverter c1 = new IndexConverter((uint)CountUniquePositions[i], r[i], sumout , w);
				c1.Offset = offset;
				for (int iIndex = 0; iIndex < CountUniquePositionNormals[i]; iIndex++) // loop each position-normal
				{
					c1.ConvertIndex();
				}
				offset += (uint)CountUniquePositions[i];
			}

			// indices to normals need to be copied in consideration of remapping 
			offset = 0;
			sumout = (uint)CountUniqueNormals.Sum();
			for (int i = 0; i < 2; i++)
			{
				IndexConverter c1 = new IndexConverter((uint)CountUniqueNormals[i], r[i], sumout, w);
				c1.Offset = offset;
				for (int iIndex = 0; iIndex < CountUniquePositionNormals[i]; iIndex++) // loop each position-normal
				{
					c1.ConvertIndex();
				}
				offset += (uint)CountUniqueNormals[i];
			}

			// now loop polygons
			offset = 0;
			sumout = (uint)CountUniquePositionNormals.Sum();
			for (int i = 0; i < 2; i++)
			{
				for (int iPoly = 0; iPoly < CountPolygons[i]; iPoly++)
				{
					// copy polygon type straight accross
					w.Write(r[i].ReadByte());
					// copy point count straight accross but keeping value
					int iCountPoints = r[i].ReadInt32();
					w.Write(iCountPoints);

					IndexConverter c1 = new IndexConverter((uint)CountUniquePositionNormals[i], r[i], sumout, w);
					c1.Offset = offset;
					for (int iPolyPoint = 0; iPolyPoint < iCountPoints; iPolyPoint++)
					{
						c1.ConvertIndex();
					}
				}
				offset += (uint)CountUniquePositionNormals[i];
			}
			w.Flush();
			// w.Close();
			_dataStream.Dispose();           
			_dataStream = result;
		}

     
		public PositionsNormalsIndicesBinaryStreamWriter AsPNIBinaryStram()
		{
			PositionsNormalsIndicesBinaryStreamWriter w = new PositionsNormalsIndicesBinaryStreamWriter();
			w.FromXbimTriangulatedModelStream(this);
			return w;
		}


		/// <summary>
		/// Used to read data from binary streams.
		/// </summary>
		private class IndexReader
		{
			private byte _IndexReaderByteSize = 0;
			protected BinaryReader _br;

			public int Size
			{
				get
				{
					return (int)_IndexReaderByteSize;
				}
			}

			public IndexReader(uint MaxSize, BinaryReader br)
			{
				_br = br;
				_IndexReaderByteSize = IndexSize(MaxSize);
			}

			protected byte IndexSize(uint MaxSize)
			{
				if (MaxSize <= 0xFF) //we will use byte for indices
					return sizeof(byte);
				else if (MaxSize <= 0xFFFF)
					return sizeof(ushort); //use  unsigned short int for indices
				else
					return sizeof(uint); //use unsigned int for indices   
			}

			public uint ReadIndex()
			{
				uint index;
				switch (_IndexReaderByteSize)
				{
					case sizeof(byte):
						index = _br.ReadByte();
						break;
					case sizeof(ushort):
						index = _br.ReadUInt16();
						break;
					default:
						index = _br.ReadUInt32();
						break;
				}
				return index;
			}
		}

		private class IndexConverter : IndexReader
		{
			private BinaryWriter _bw;
			private byte _IndexWriterByteSize = 0;
			public uint Offset { get; set; }

			public IndexConverter(uint MaxSizeReader, BinaryReader br, uint MaxSizeWriter, BinaryWriter bw)
				: base(MaxSizeReader, br)
			{
				_IndexWriterByteSize = IndexSize(MaxSizeWriter);
				_bw = bw;
			}

			public void WriteIndex(uint Index)
			{
				switch (_IndexWriterByteSize)
				{
					case sizeof(byte):
						_bw.Write((byte)Index);
						break;
					case sizeof(ushort):
						_bw.Write((ushort)Index);
						break;
					default:
						_bw.Write((uint)Index);
						break;
				}
			}

			internal void ConvertIndex()
			{
				WriteIndex(ReadIndex() + Offset);
			}
		}

		// conversion to IXbimTriangulatesToPositionsIndices
		//
		public void Build<TGeomType>(TGeomType builder) where TGeomType : IXbimTriangulatesToPositionsIndices, new() 
		{
			_dataStream.Seek(0, SeekOrigin.Begin);
			BinaryReader br = new BinaryReader(_dataStream);
			
			builder.BeginBuild();
			if (! this.IsEmpty) 
				 Build(builder, br);
			// children have been removed
			builder.EndBuild();
		}

		private void Build<TGeomType>(TGeomType builder, BinaryReader br) where TGeomType : IXbimTriangulatesToPositionsIndices, new()
		{
			uint numPositions = br.ReadUInt32();
			uint numNormals = br.ReadUInt32();
			uint numUniques = br.ReadUInt32();
			uint numTriangles = br.ReadUInt32();
			uint numPolygons = br.ReadUInt32();

			IndexReader PositionReader = new IndexReader(numPositions, br);
			IndexReader NormalsReader = new IndexReader(numNormals, br);
			IndexReader UniquesReader = new IndexReader(numUniques, br);


			// coordinates of positions
			//
			builder.BeginPositions(numPositions);
			for (uint i = 0; i < numPositions; i++)
			{
				double x = br.ReadSingle();
				double y = br.ReadSingle();
				double z = br.ReadSingle();
				builder.AddPosition(new XbimPoint3D(x, y, z));
			}
			builder.EndPositions();

			// skips normal coordinates
			br.BaseStream.Seek(numNormals * sizeof(float) * 3, SeekOrigin.Current);

			// prepares local array of point coordinates.
			uint[] UniqueToPosition = new uint[numUniques];
			for (uint i = 0; i < numUniques; i++)
			{
				uint readposition = PositionReader.ReadIndex();
				UniqueToPosition[i] = readposition;
			}

			// skips normal indices
			br.BaseStream.Seek(numUniques * NormalsReader.Size, SeekOrigin.Current);


			builder.BeginPolygons(numTriangles, numPolygons);
			for (uint p = 0; p < numPolygons; p++)
			{
				// set the state
				TriangleType meshType = (TriangleType)br.ReadByte();
				uint indicesCount = br.ReadUInt32();
				builder.BeginPolygon(meshType, indicesCount);
				//get the triangles
				for (uint i = 0; i < indicesCount; i++)
				{
					uint iUi = UniquesReader.ReadIndex();
					builder.AddTriangleIndex(UniqueToPosition[iUi]);
				}
				builder.EndPolygon();
			}
			builder.EndPolygons();
		}


        /// <summary>
        /// Builds a triangulated mesh with normals, appends points etc t the end of the existing mesh
        /// </summary>
        /// <typeparam name="TGeomType"></typeparam>
        /// <param name="builder"></param>
        /// <param name="transform"></param>
        /// <param name="modelId"></param>
        /// <returns>The fragment defining the piece of the mesh built with this operation
        /// If there is no data an empty fragment is returned, if the mesh is goinng to excees the size of a an unsigned short
        /// then the data is not added and a fragement with zero number of points is returned and 
        /// a start position that is equal to the length of the mesh. The Entity Label is also sent to int.MinValue</returns>
        public XbimMeshFragment BuildWithNormals<TGeomType>(TGeomType builder, XbimMatrix3D transform, short modelId=0) where TGeomType : IXbimTriangulatesToPositionsNormalsIndices
        {
            _dataStream.Seek(0, SeekOrigin.Begin);
            BinaryReader br = new BinaryReader(_dataStream);
           
            if (!IsEmpty) // has data 
            {
                builder.BeginBuild();
                XbimMeshFragment fragment = new XbimMeshFragment(builder.PositionCount,builder.TriangleIndexCount, modelId);
                if (!BuildWithNormals(builder, br, transform))
                    fragment.EntityLabel = -1; //set the entity label to indicate failure
                fragment.EndPosition = builder.PositionCount-1;
                fragment.EndTriangleIndex = builder.TriangleIndexCount-1;
                builder.EndBuild();
                return fragment;
            } 
            else
                return default(XbimMeshFragment);
        }


        /// <summary>
        /// If adding the data to the mesh causes the mesh to exceed the max size of ushort.MaxSize
        /// the data is not added and false is returned.
        /// </summary>
        /// <typeparam name="TGeomType"></typeparam>
        /// <param name="builder"></param>
        /// <param name="br"></param>
        /// <param name="transform"></param>
        /// <returns></returns>
        private bool BuildWithNormals<TGeomType>(TGeomType builder, BinaryReader br, XbimMatrix3D transform) where TGeomType : IXbimTriangulatesToPositionsNormalsIndices
		{
           
			uint numPositions = br.ReadUInt32();
            //if we the mesh is smaller that 64K then try and add it to this mesh, if it is bigger than 65K we just have to stake what we can
            //if (numPositions < ushort.MaxValue && builder.PositionCount > 0 && (builder.PositionCount + numPositions >= ushort.MaxValue)) //we cannot build meshes bigger than this and pass them through to standard graphics buffers
            //    return false;        
			uint numNormals = br.ReadUInt32();
			uint numUniques = br.ReadUInt32();
			uint numTriangles = br.ReadUInt32();
			uint numPolygons = br.ReadUInt32();

			IndexReader PositionReader = new IndexReader(numPositions, br);
			IndexReader NormalsReader = new IndexReader(numNormals, br);
			IndexReader UniquesReader = new IndexReader(numUniques, br);

			float[,] pos = new float[numPositions,3];
			float[,] nrm;	
			nrm = new float[numNormals, 3];
			
			// coordinates of positions
			//
			for (uint i = 0; i < numPositions; i++)
			{
				pos[i, 0] = br.ReadSingle();
				pos[i, 1] = br.ReadSingle();
				pos[i, 2] = br.ReadSingle();
			}
			// dimensions of normals
			//
			for (uint i = 0; i < numNormals; i++)
			{
				nrm[i, 0] = br.ReadSingle();
				nrm[i, 1] = br.ReadSingle();
				nrm[i, 2] = br.ReadSingle();
			}

			// loop twice for how many indices to create the point/normal combinations.
			builder.BeginPoints(numUniques);
            if (transform.IsIdentity)
            {
                for (uint i = 0; i < numUniques; i++)
                {
                    uint readpositionI = PositionReader.ReadIndex();
                    builder.AddPosition(
                        new XbimPoint3D(pos[readpositionI, 0], pos[readpositionI, 1], pos[readpositionI, 2]));
                }
                for (uint i = 0; i < numUniques; i++)
                {
                    uint readnormalI = NormalsReader.ReadIndex();
                    builder.AddNormal(
                        new XbimVector3D(nrm[readnormalI, 0], nrm[readnormalI, 1], nrm[readnormalI, 2])
                        );
                }
            }
            else
            {
                for (uint i = 0; i < numUniques; i++)
                {
                    uint readpositionI = PositionReader.ReadIndex();
                    var tfdPosition = transform.Transform(new XbimPoint3D(pos[readpositionI, 0], pos[readpositionI, 1], pos[readpositionI, 2]));
                    builder.AddPosition(tfdPosition);
                }
                for (uint i = 0; i < numUniques; i++)
                {
                    // todo: use a quaternion extracted from the matrix instead
                    //
                    uint readnormalI = NormalsReader.ReadIndex();
                    var origNormal = new XbimVector3D(nrm[readnormalI, 0], nrm[readnormalI, 1], nrm[readnormalI, 2]);
                    XbimVector3D v = transform.Transform(origNormal);
                    v = v.Normalized();
                    builder.AddNormal(v);
                }
            }
           
			builder.EndPoints(); //point/normal combinations completed

			builder.BeginPolygons(numTriangles, numPolygons);
			for (uint p = 0; p < numPolygons; p++)
			{
				// set the state
				TriangleType meshType = (TriangleType)br.ReadByte();
				uint indicesCount = br.ReadUInt32();
				builder.BeginPolygon(meshType, indicesCount);
				//get the triangles
				for (uint i = 0; i < indicesCount; i++)
				{
					builder.AddTriangleIndex(UniquesReader.ReadIndex());
				}
				builder.EndPolygon();
			}
			builder.EndPolygons();
            return true;
		}

		public void BuildPNI<TGeomType>(TGeomType builder) where TGeomType : IXbimTriangulatesToSimplePositionsNormalsIndices, new()
		{
			_dataStream.Seek(0, SeekOrigin.Begin);
			BinaryReader br = new BinaryReader(_dataStream);

			uint numPositions = br.ReadUInt32();
			uint numNormals = br.ReadUInt32();
			uint numUniques = br.ReadUInt32();
			uint numTriangles = br.ReadUInt32();
			uint numPolygons = br.ReadUInt32();

			builder.BeginBuild(numUniques, numTriangles);
			

			IndexReader PositionReader = new IndexReader(numPositions, br);
			IndexReader NormalsReader = new IndexReader(numNormals, br);
			IndexReader UniquesReader = new IndexReader(numUniques, br);

			float[,] pos = new float[numPositions, 3];
			float[,] nrm = new float[numNormals, 3];
			uint[,] uniques = new uint[numUniques, 2];
			
			// coordinates of positions
			//
			for (uint i = 0; i < numPositions; i++)
			{
				pos[i, 0] = br.ReadSingle();
				pos[i, 1] = br.ReadSingle();
				pos[i, 2] = br.ReadSingle();
			}
			// dimensions of normals
			//
			for (uint i = 0; i < numNormals; i++)
			{
				nrm[i, 0] = br.ReadSingle();
				nrm[i, 1] = br.ReadSingle();
				nrm[i, 2] = br.ReadSingle();
			}

			// loop twice for how many indices to create the point/normal combinations.
			for (uint i = 0; i < numUniques; i++)
			{
				uint readpositionI = PositionReader.ReadIndex();
				uniques[i, 0] = readpositionI;
					
			}
			for (uint i = 0; i < numUniques; i++)
			{
				uint readnormalI = NormalsReader.ReadIndex();
				uniques[i, 1] = readnormalI;
			}

			builder.BeginPoints(numUniques);

			for (uint i = 0; i < numUniques; i++)
			{
				builder.AddPoint(
					pos[uniques[i, 0], 0], // uniques[i, 0] is the point index
					pos[uniques[i, 0], 1],
					pos[uniques[i, 0], 2],
					nrm[uniques[i, 1], 0], // uniques[i, 1] is the normal index
					nrm[uniques[i, 1], 1],
					nrm[uniques[i, 1], 2]
					);
			}

			builder.BeginTriangles(numTriangles); //point/normal combinations completed

			
			for (uint p = 0; p < numPolygons; p++)
			{
				// set the state
				TriangleType meshType = (TriangleType)br.ReadByte();
				uint indicesCount = br.ReadUInt32();

				uint _pointTally = 0;
				uint _fanStartIndex = 0;
				uint _previousToLastIndex = 0;
				uint _lastIndex = 0;
				
				//get the triangles
				for (uint i = 0; i < indicesCount; i++)
				{
					uint index = UniquesReader.ReadIndex();

					if (_pointTally == 0)
						_fanStartIndex = index;
					if (_pointTally < 3) //first time
					{
						builder.AddTriangleIndex(index);
					}
					else
					{
						switch (meshType)
						{
							case TriangleType.GL_Triangles://      0x0004
								builder.AddTriangleIndex(index);
								// _meshGeometry.Positions.Add(_points[(int)index]);
								break;
							case TriangleType.GL_Triangles_Strip:// 0x0005
								if (_pointTally % 2 == 0)
								{
									builder.AddTriangleIndex(_previousToLastIndex);
									builder.AddTriangleIndex(_lastIndex);
								}
								else
								{
									builder.AddTriangleIndex(_lastIndex);
									builder.AddTriangleIndex(_previousToLastIndex);
								}
								builder.AddTriangleIndex(index);
								break;
							case TriangleType.GL_Triangles_Fan://   0x0006
								builder.AddTriangleIndex(_fanStartIndex);
								builder.AddTriangleIndex(_lastIndex);                                
								builder.AddTriangleIndex(index);
								break;
							default:
								break;
						}
					}
					_previousToLastIndex = _lastIndex;
					_lastIndex = index;
					_pointTally++;
				}
			}
			builder.EndBuild();
		}
	}
}
