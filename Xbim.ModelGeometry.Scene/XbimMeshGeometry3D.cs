using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Common.Exceptions;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;


namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// This class provide support for geoemtry triangulated neshes
    /// </summary>
    public class XbimMeshGeometry3D : IXbimMeshGeometry3D
    {
        object meshLock = new object();
        const int DefaultSize = 0x4000;
        public List<XbimPoint3D> Positions;
        public List<XbimVector3D> Normals;
        public List<Int32> TriangleIndices;

        XbimMeshFragmentCollection _meshes = new XbimMeshFragmentCollection();
        List<XbimPoint3D> _points = new List<XbimPoint3D>(512);
        TriangleType _meshType;
        uint _previousToLastIndex;
        uint _lastIndex;
        uint _pointTally;
        uint _fanStartIndex;
        uint _indexOffset;

        public void ReportGeometryTo(StringBuilder sb)
        {
            var i = 0;
            var pEn = Positions.GetEnumerator();
            var nEn = Normals.GetEnumerator();
            while (pEn.MoveNext() && nEn.MoveNext())
            {
                var p = pEn.Current;
                var n = nEn.Current;
                sb.AppendFormat("{0} pos: {1} nrm:{2}\r\n", i++, p, n);
            }

            i = 0;
            sb.AppendLine("Triangles:");
            foreach (var item in TriangleIndices)
            {
                sb.AppendFormat("{0}, ", item);
                i++;
                if (i % 3 == 0)
                {
                    sb.AppendLine();
                }
            }
        }

        public XbimMeshGeometry3D(int size)
        {
            Positions = new List<XbimPoint3D>(size);
            Normals = new List<XbimVector3D>(size);
            TriangleIndices = new List<Int32>(size * 3);
        }

        public XbimMeshGeometry3D()
            : this(DefaultSize)
        {

        }

        /// <summary>
        /// Reads an ascii string of Xbim mesh geometry data
        /// </summary>
        /// <param name="data"></param>
        /// <param name="trans">An optional transformation</param>
        /// <returns></returns>
        public bool Read(String data, XbimMatrix3D? trans = null)
        {
            var version = 2; //we are at at least verson 2 now
            var q = new XbimQuaternion();
            if (trans.HasValue)
                q = trans.Value.GetRotationQuaternion();
            using (var sr = new StringReader(data))
            {

                var vertexList = new List<XbimPoint3D>(); //holds the actual positions of the vertices in this data set in the mesh
                var normalList = new List<XbimVector3D>(); //holds the actual normals of the vertices in this data set in the mesh
                String line;
                // Read and display lines from the data until the end of
                // the data is reached.

                while ((line = sr.ReadLine()) != null)
                {

                    var tokens = line.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                    if (tokens.Length > 1) //we need a command and some data
                    {
                        var command = tokens[0].Trim().ToUpper();
                        switch (command)
                        {
                            case "P":
                                version = Int32.Parse(tokens[1]);
                                var pointCount = 512;
                                //var faceCount = 128;
                                //var triangleCount = 256;
                                var normalCount = 512;
                                if (tokens.Length > 1) pointCount = Int32.Parse(tokens[2]);
                               // if (tokens.Length > 2) faceCount = Int32.Parse(tokens[3]);
                               // if (tokens.Length > 3) triangleCount = Int32.Parse(tokens[4]);
                                //version 2 of the string format uses packed normals
                                if (version < 2 && tokens.Length > 4) normalCount = Int32.Parse(tokens[5]);
                                vertexList = new List<XbimPoint3D>(pointCount);
                                normalList = new List<XbimVector3D>(normalCount);
                                break;
                            case "V": //process vertices
                                for (var i = 1; i < tokens.Length; i++)
                                {
                                    var xyz = tokens[i].Split(',');
                                    var p = new XbimPoint3D(Convert.ToDouble(xyz[0], CultureInfo.InvariantCulture),
                                                                      Convert.ToDouble(xyz[1], CultureInfo.InvariantCulture),
                                                                      Convert.ToDouble(xyz[2], CultureInfo.InvariantCulture));
                                    if (trans.HasValue)
                                        p = trans.Value.Transform(p);
                                    vertexList.Add(p);
                                }
                                break;
                            case "N": //processes normals
                                for (var i = 1; i < tokens.Length; i++)
                                {
                                    var xyz = tokens[i].Split(',');
                                    var v = new XbimVector3D(Convert.ToDouble(xyz[0], CultureInfo.InvariantCulture),
                                                                       Convert.ToDouble(xyz[1], CultureInfo.InvariantCulture),
                                                                       Convert.ToDouble(xyz[2], CultureInfo.InvariantCulture));
                                    normalList.Add(v);
                                }
                                break;
                            case "T": //process triangulated meshes
                                var currentNormal = XbimVector3D.Zero;
                                //each time we start a new mesh face we have to duplicate the vertices to ensure that we get correct shading of planar and non planar faces
                                var writtenVertices = new Dictionary<int, int>();

                                for (var i = 1; i < tokens.Length; i++)
                                {
                                    var triangleIndices = tokens[i].Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                                    if (triangleIndices.Length != 3) throw new Exception("Invalid triangle definition");
                                    for (var t = 0; t < 3; t++)
                                    {
                                        var indexNormalPair = triangleIndices[t].Split(new[] { '/' }, StringSplitOptions.RemoveEmptyEntries);

                                        if (indexNormalPair.Length > 1) //we have a normal defined
                                        {
                                            var normalStr = indexNormalPair[1].Trim();
                                            if (version < 2)
                                            {
                                                switch (normalStr)
                                                {
                                                    case "F": //Front
                                                        currentNormal = new XbimVector3D(0, -1, 0);
                                                        break;
                                                    case "B": //Back
                                                        currentNormal = new XbimVector3D(0, 1, 0);
                                                        break;
                                                    case "L": //Left
                                                        currentNormal = new XbimVector3D(-1, 0, 0);
                                                        break;
                                                    case "R": //Right
                                                        currentNormal = new XbimVector3D(1, 0, 0);
                                                        break;
                                                    case "U": //Up
                                                        currentNormal = new XbimVector3D(0, 0, 1);
                                                        break;
                                                    case "D": //Down
                                                        currentNormal = new XbimVector3D(0, 0, -1);
                                                        break;
                                                    default: //it is an index number
                                                        var normalIndex = int.Parse(indexNormalPair[1]);
                                                        currentNormal = normalList[normalIndex];
                                                        break;
                                                }
                                            }
                                            else
                                            {
                                                var normalIndex = ushort.Parse(indexNormalPair[1]);  
                                                var packedNormal = new XbimPackedNormal(normalIndex);
                                                currentNormal = packedNormal.Normal;
                                            }
                                            if (trans.HasValue)
                                            {
                                                XbimVector3D v;
                                                XbimQuaternion.Transform(ref currentNormal, ref q, out v);
                                                currentNormal = v;

                                            }
                                        }

                                        //now add the index
                                        var index = int.Parse(indexNormalPair[0]);

                                        int alreadyWrittenAt; 
                                        if (!writtenVertices.TryGetValue(index, out alreadyWrittenAt)) //if we haven't  written it in this mesh pass, add it again unless it is the first one which we know has been written
                                        {
                                            //all vertices will be unique and have only one normal
                                            writtenVertices.Add(index, PositionCount);
                                            TriangleIndices.Add(PositionCount);
                                            Positions.Add(vertexList[index]);
                                            Normals.Add(currentNormal); 
                                        }
                                        else //just add the index reference
                                        {
                                            TriangleIndices.Add(alreadyWrittenAt);
                                        }
                                    }
                                }

                                break;
                            case "F":
                                break;
                            default:
                                throw new Exception("Invalid Geometry Command");

                        }
                    }
                }
            }
            return true;
        }

       

        #region standard calls

        private void Init()
        {
            _indexOffset = (uint)Positions.Count;
        }

        private void StandardBeginPolygon(TriangleType meshType)
        {
            _meshType = meshType;
            _pointTally = 0;
            _previousToLastIndex = 0;
            _lastIndex = 0;
            _fanStartIndex = 0;
        }
        #endregion

        #region IXbimTriangulatesToPointsIndices

        void IXbimTriangulatesToPositionsIndices.BeginBuild()
        {
            Init();
        }

        void IXbimTriangulatesToPositionsIndices.BeginPositions(uint numPoints)
        {
            _points = new List<XbimPoint3D>((int)numPoints);
        }

        void IXbimTriangulatesToPositionsIndices.AddPosition(XbimPoint3D xbimPoint3D)
        {
            _points.Add(xbimPoint3D);
        }

        void IXbimTriangulatesToPositionsIndices.EndPositions()
        {
        }

        void IXbimTriangulatesToPositionsIndices.BeginPolygons(uint totalNumberTriangles, uint numPolygons)
        {

        }

        void IXbimTriangulatesToPositionsIndices.BeginPolygon(TriangleType meshType, uint indicesCount)
        {
            StandardBeginPolygon(meshType);
        }

        void IXbimTriangulatesToPositionsIndices.AddTriangleIndex(uint index)
        {
            if (_pointTally == 0)
                _fanStartIndex = index;
            if (_pointTally < 3) //first time
            {
                TriangleIndices.Add(Positions.Count);
                Positions.Add(_points[(int)index]);
            }
            else
            {
                switch (_meshType)
                {
                    case TriangleType.GL_Triangles://      0x0004
                        TriangleIndices.Add(Positions.Count);
                        Positions.Add(_points[(int)index]);
                        break;
                    case TriangleType.GL_Triangles_Strip:// 0x0005
                        if (_pointTally % 2 == 0)
                        {
                            TriangleIndices.Add(Positions.Count);
                            Positions.Add(_points[(int)_previousToLastIndex]);
                            TriangleIndices.Add(Positions.Count);
                            Positions.Add(_points[(int)_lastIndex]);
                        }
                        else
                        {
                            TriangleIndices.Add(Positions.Count);
                            Positions.Add(_points[(int)_lastIndex]);
                            TriangleIndices.Add(Positions.Count);
                            Positions.Add(_points[(int)_previousToLastIndex]);
                        }
                        TriangleIndices.Add(Positions.Count);
                        Positions.Add(_points[(int)index]);
                        break;
                    case TriangleType.GL_Triangles_Fan://   0x0006
                        TriangleIndices.Add(Positions.Count);
                        Positions.Add(_points[(int)_fanStartIndex]);
                        TriangleIndices.Add(Positions.Count);
                        Positions.Add(_points[(int)_lastIndex]);
                        TriangleIndices.Add(Positions.Count);
                        Positions.Add(_points[(int)index]);
                        break;
                }
            }
            _previousToLastIndex = _lastIndex;
            _lastIndex = index;
            _pointTally++;
        }

        void IXbimTriangulatesToPositionsIndices.EndPolygon()
        {

        }

        void IXbimTriangulatesToPositionsIndices.EndPolygons()
        {

        }

        void IXbimTriangulatesToPositionsIndices.EndBuild()
        {

        }
        #endregion

        #region IXbimTriangulatesToPositionsNormalsIndices

        void IXbimTriangulatesToPositionsNormalsIndices.BeginBuild()
        {
            Init();
        }

        void IXbimTriangulatesToPositionsNormalsIndices.BeginPoints(uint numPoints)
        {

        }

        void IXbimTriangulatesToPositionsNormalsIndices.AddPosition(XbimPoint3D xbimPoint3D)
        {
            Positions.Add(xbimPoint3D);
        }

        void IXbimTriangulatesToPositionsNormalsIndices.AddNormal(XbimVector3D normal)
        {
            Normals.Add(normal);
        }

        void IXbimTriangulatesToPositionsNormalsIndices.EndPoints()
        {
            // purposely empty
        }

        void IXbimTriangulatesToPositionsNormalsIndices.BeginPolygons(uint totalNumberTriangles, uint numPolygons)
        {

        }

        void IXbimTriangulatesToPositionsNormalsIndices.BeginPolygon(TriangleType meshType, uint indicesCount)
        {
            StandardBeginPolygon(meshType);
        }

        private int Offset(uint index)
        {
            return (int)(index + _indexOffset);
        }

        void IXbimTriangulatesToPositionsNormalsIndices.AddTriangleIndex(uint index)
        {

            if (_pointTally == 0)
                _fanStartIndex = index;
            if (_pointTally < 3) //first time
            {
                TriangleIndices.Add(Offset(index));
                // _meshGeometry.Positions.Add(_points[(int)index]);
            }
            else
            {
                switch (_meshType)
                {
                    case TriangleType.GL_Triangles://      0x0004
                        TriangleIndices.Add(Offset(index));
                        break;
                    case TriangleType.GL_Triangles_Strip:// 0x0005
                        if (_pointTally % 2 == 0)
                        {
                            TriangleIndices.Add(Offset(_previousToLastIndex));
                            TriangleIndices.Add(Offset(_lastIndex));
                        }
                        else
                        {
                            TriangleIndices.Add(Offset(_lastIndex));
                            TriangleIndices.Add(Offset(_previousToLastIndex));
                        }
                        TriangleIndices.Add(Offset(index));
                        break;
                    case TriangleType.GL_Triangles_Fan://   0x0006
                        TriangleIndices.Add(Offset(_fanStartIndex));
                        TriangleIndices.Add(Offset(_lastIndex));
                        TriangleIndices.Add(Offset(index));
                        break;
                }
            }
            _previousToLastIndex = _lastIndex;
            _lastIndex = index;
            _pointTally++;
        }

        void IXbimTriangulatesToPositionsNormalsIndices.EndPolygon()
        {
            // purposely empty
        }

        void IXbimTriangulatesToPositionsNormalsIndices.EndPolygons()
        {
            // purposely empty
        }

        void IXbimTriangulatesToPositionsNormalsIndices.EndBuild()
        {
            // purposely empty
        }
        #endregion

        //adds the content of the toAdd to this, it is added as a single mesh fragment, any meshes in toAdd are lost
        public void Add(IXbimMeshGeometry3D toAdd, int entityLabel, Type ifcType, short modelId)
        {
            var startPosition = Positions.Count;
            var fragment = new XbimMeshFragment(startPosition, TriangleIndexCount, modelId);
            Positions.AddRange(toAdd.Positions);
            Normals.AddRange(toAdd.Normals);
            foreach (var idx in toAdd.TriangleIndices)
                TriangleIndices.Add(idx + startPosition);
            fragment.EndPosition = PositionCount - 1;
            fragment.EndTriangleIndex = TriangleIndexCount - 1;
            fragment.EntityLabel = entityLabel;
            throw new NotImplementedException();//need to fix this
           // fragment.EntityTypeId = IfcMetaData.IfcTypeId(ifcType);
            //_meshes.Add(fragment);
        }


        public int PositionCount
        {
            get { return Positions.Count; }
        }

        public int TriangleIndexCount
        {
            get { return TriangleIndices.Count; }
        }

        IEnumerable<XbimPoint3D> IXbimMeshGeometry3D.Positions
        {
            get { return Positions; }
            set { Positions = new List<XbimPoint3D>(value); }
        }

        IEnumerable<XbimVector3D> IXbimMeshGeometry3D.Normals
        {
            get { return Normals; }
            set { Normals = new List<XbimVector3D>(value); }
        }


        IList<int> IXbimMeshGeometry3D.TriangleIndices
        {
            get { return TriangleIndices; }
            set { TriangleIndices = new List<int>(value); }
        }


        public XbimMeshFragmentCollection Meshes
        {
            get { return _meshes; }
            set
            {
                _meshes = new XbimMeshFragmentCollection(value);
            }
        }

        /// <summary>
        /// Appends a geometry data object to the Mesh, returns false if the mesh would become too big and needs splitting
        /// </summary>
        /// <param name="geometryMeshData"></param>
        /// <param name="modelId"></param>
        public bool Add(XbimGeometryData geometryMeshData, short modelId = 0)
        {
            var transform = XbimMatrix3D.FromArray(geometryMeshData.DataArray2);
            if (geometryMeshData.GeometryType == XbimGeometryType.TriangulatedMesh)
            {
                var strm = new XbimTriangulatedModelStream(geometryMeshData.ShapeData);
                var fragment = strm.BuildWithNormals(this, transform, modelId);
                if (fragment.EntityLabel == -1) //nothing was added due to size being exceeded
                    return false;
                fragment.EntityLabel = geometryMeshData.IfcProductLabel;
                fragment.EntityTypeId = geometryMeshData.IfcTypeId;
                _meshes.Add(fragment);
            }
            else if (geometryMeshData.GeometryType == XbimGeometryType.BoundingBox)
            {
                var r3D = XbimRect3D.FromArray(geometryMeshData.ShapeData);
                throw new NotImplementedException();//need to fix this
               // Add(MakeBoundingBox(r3D, transform), geometryMeshData.IfcProductLabel, IfcMetaData.GetType(geometryMeshData.IfcTypeId), modelId);
            }
            else
                throw new XbimException("Illegal geometry type found");
            return true;
        }

        /// <summary>
        /// Moves the content of this mesh to the other
        /// </summary>
        /// <param name="toMesh"></param>
        public void MoveTo(IXbimMeshGeometry3D toMesh)
        {
            if (_meshes.Any()) //if no meshes nothing to move
            {
                toMesh.BeginUpdate();
                toMesh.Positions = Positions; Positions.Clear();
                toMesh.Normals = Normals; Normals.Clear();
                toMesh.TriangleIndices = TriangleIndices; TriangleIndices.Clear();
                toMesh.Meshes = Meshes; _meshes.Clear();
                toMesh.EndUpdate();
            }
        }


        public void BeginUpdate()
        {

        }

        public void EndUpdate()
        {

        }


        public IXbimMeshGeometry3D GetMeshGeometry3D(XbimMeshFragment frag)
        {
            var m3D = new XbimMeshGeometry3D();
            for (var i = frag.StartPosition; i <= frag.EndPosition; i++)
            {
                m3D.Positions.Add(Positions[i]);
                m3D.Normals.Add(Normals[i]);
            }
            for (var i = frag.StartTriangleIndex; i <= frag.EndTriangleIndex; i++)
            {
                m3D.TriangleIndices.Add(TriangleIndices[i] - frag.StartPosition);
            }
            return m3D;
        }

        /// <summary>
        /// Adds a geometry mesh to this, includes all mesh fragments
        /// </summary>
        /// <param name="geom"></param>
        public void Add(IXbimMeshGeometry3D geom)
        {
            if (geom.Positions.Any()) //if no positions nothing to add
            {
                BeginUpdate();
                var startPos = Positions.Count;
                var startIndices = TriangleIndices.Count;
                Positions.AddRange(geom.Positions);
                Normals.AddRange(geom.Normals);
                foreach (var indices in geom.TriangleIndices)
                    TriangleIndices.Add(indices + startPos);
                foreach (var fragment in geom.Meshes)
                {
                    fragment.Offset(startPos, startIndices);
                    Meshes.Add(fragment);
                }

                EndUpdate();
            }
        }

        public byte[] ToByteArray()
        {
            //calculate the indices count
            var pointCount = 0;
            foreach (var mesh in _meshes)
                pointCount += mesh.EndTriangleIndex - mesh.StartTriangleIndex + 1;
            var bytes = new byte[pointCount * ((6 * sizeof(float)) + sizeof(int))]; //max size of data stream
            var ms = new MemoryStream(bytes);
            var bw = new BinaryWriter(ms);
            foreach (var mesh in _meshes)
            {
                var label = mesh.EntityLabel;
                label = ((label & 0xff) << 24) + ((label & 0xff00) << 8) + ((label & 0xff0000) >> 8) + ((label >> 24) & 0xff);
                for (var i = mesh.StartTriangleIndex; i <= mesh.EndTriangleIndex; i++)
                {
                    var pt = Positions[TriangleIndices[i]];
                    var n = Normals[TriangleIndices[i]];
                    bw.Write(pt.X); bw.Write(pt.Y); bw.Write(pt.Z);
                    bw.Write(n.X); bw.Write(n.Y); bw.Write(n.Z);
                    bw.Write(label);
                }
            }
            return bytes;
        }


        public XbimRect3D GetBounds()
        {
            var first = true;
            var boundingBox = XbimRect3D.Empty;
            foreach (var pos in Positions)
            {
                if (first)
                {
                    boundingBox = new XbimRect3D(pos);
                    first = false;
                }
                else
                    boundingBox.Union(pos);

            }
            return boundingBox;
        }


        /// <summary>
        /// Adds the geometry to the mesh for the given product, returns the mesh fragment details
        /// </summary>
        /// <param name="geometryModel">Geometry to add</param>
        /// <param name="product">The product the geometry represents (this may be a partial representation)</param>
        /// <param name="transform">Transform the geometry to a new location or rotation</param>
        /// <param name="deflection">Deflection for triangulating curves, if null default defelction for the model is used</param>
        /// <param name="modelId">An optional model ID</param>
        public XbimMeshFragment Add(IXbimGeometryModel geometryModel, IIfcProduct product, XbimMatrix3D transform, 
            double? deflection = null, short modelId = 0)
        {
            return geometryModel.MeshTo(this, product, transform, deflection ?? product.Model.ModelFactors.DeflectionTolerance);
        }



        public void Add(string mesh, Type productType, int productLabel, int geometryLabel, XbimMatrix3D? transform, short modelId)
        {
            throw new NotImplementedException();//need to fix this
         //   Add(mesh, IfcMetaData.IfcTypeId(productType), productLabel, geometryLabel, transform, modelId);

        }

        public void Add(string mesh, short productTypeId, int productLabel, int geometryLabel, XbimMatrix3D? transform, short modelId)
        {
            lock (meshLock)
            {
                var frag = new XbimMeshFragment(PositionCount, TriangleIndexCount, productTypeId, productLabel, geometryLabel, modelId);
                Read(mesh, transform);
                frag.EndPosition = PositionCount - 1;
                frag.EndTriangleIndex = TriangleIndexCount - 1;
                _meshes.Add(frag);
            }

        }




    }
}