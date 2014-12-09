using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Common.Exceptions;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.Kernel;
using Xbim.IO;
using Xbim.XbimExtensions;
using XbimGeometry.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// This class provide support for geoemtry triangulated neshes
    /// </summary>
    public class XbimMeshGeometry3D : IXbimMeshGeometry3D
    {
        object meshLock = new object();
        const int defaultSize = 0x4000;
        public List<XbimPoint3D> Positions;
        public List<XbimVector3D> Normals;
        public List<Int32> TriangleIndices;

        XbimMeshFragmentCollection meshes = new XbimMeshFragmentCollection();
        List<XbimPoint3D> _points = new List<XbimPoint3D>(512);
        TriangleType _meshType;
        uint _previousToLastIndex;
        uint _lastIndex;
        uint _pointTally;
        uint _fanStartIndex;
        uint indexOffset;

        public void ReportGeometryTo(StringBuilder sb)
        {
            int i = 0;
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

        public XbimMeshGeometry3D() :this(defaultSize)
        {

        }

        /// <summary>
        /// Reads an ascii string of Xbim mesh geometry data
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        public bool Read(String data, XbimMatrix3D? trans = null)
        {
            XbimQuaternion q = new XbimQuaternion();
            if (trans.HasValue)
                q = trans.Value.GetRotationQuaternion();
            using (StringReader sr = new StringReader(data))
            {

                List<XbimPoint3D> vertexList = new List<XbimPoint3D>(); //holds the actual positions of the vertices in this data set in the mesh
                List<XbimVector3D> normalList = new List<XbimVector3D>(); //holds the actual normals of the vertices in this data set in the mesh
                String line;
                // Read and display lines from the data until the end of
                // the data is reached.
              
                while ((line = sr.ReadLine()) != null)
                {
                   
                    string[] tokens = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                    if (tokens.Length > 1) //we need a command and some data
                    {
                        string command = tokens[0].Trim().ToUpper();
                        switch (command)
                        {
                            case "P":
                                int pointCount = 512;
                                int faceCount = 128;
                                int triangleCount = 256;
                                int normalCount = 512;
                                if (tokens.Length > 1) pointCount = Int32.Parse(tokens[2]);
                                if (tokens.Length > 2) faceCount = Int32.Parse(tokens[3]);
                                if (tokens.Length > 3) triangleCount = Int32.Parse(tokens[4]);
                                if (tokens.Length > 4) normalCount = Int32.Parse(tokens[5]);
                                vertexList = new List<XbimPoint3D>(pointCount);
                                normalList = new List<XbimVector3D>(normalCount);
                                break;
                            case "V": //process vertices
                                for (int i = 1; i < tokens.Length; i++)
                                {
                                   string[] xyz = tokens[i].Split(',');
                                   XbimPoint3D p = new XbimPoint3D(Convert.ToDouble(xyz[0], CultureInfo.InvariantCulture),
                                                                     Convert.ToDouble(xyz[1], CultureInfo.InvariantCulture),
                                                                     Convert.ToDouble(xyz[2], CultureInfo.InvariantCulture));
                                   if (trans.HasValue)
                                       p = trans.Value.Transform(p);
                                   vertexList.Add(p);
                                }
                                break;
                            case "N": //processes normals
                                for (int i = 1; i < tokens.Length; i++)
                                {
                                    string[] xyz = tokens[i].Split(',');
                                    XbimVector3D v = new XbimVector3D(Convert.ToDouble(xyz[0], CultureInfo.InvariantCulture),
                                                                       Convert.ToDouble(xyz[1], CultureInfo.InvariantCulture),
                                                                       Convert.ToDouble(xyz[2], CultureInfo.InvariantCulture));
                                    normalList.Add(v);
                                }
                                break;
                            case "T": //process triangulated meshes
                                XbimVector3D currentNormal = XbimVector3D.Zero;
                                //each time we start a new mesh face we have to duplicate the vertices to ensure that we get correct shading of planar and non planar faces
                                Dictionary<int, int> writtenVertices = new Dictionary<int, int>();

                                for (int i = 1; i < tokens.Length; i++)
                                {
                                    string[] triangleIndices = tokens[i].Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                                    if (triangleIndices.Length != 3) throw new Exception("Invalid triangle definition");
                                    for (int t = 0; t < 3; t++)
                                    {
                                        string[] indexNormalPair = triangleIndices[t].Split(new char[] { '/' }, StringSplitOptions.RemoveEmptyEntries);

                                        if (indexNormalPair.Length > 1) //we have a normal defined
                                        {
                                            string normalStr = indexNormalPair[1].Trim();
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
                                                    int normalIndex = int.Parse(indexNormalPair[1]);
                                                    currentNormal = normalList[normalIndex];
                                                    break;
                                            }
                                            if (trans.HasValue)
                                            {
                                                XbimVector3D v;
                                                XbimQuaternion.Transform(ref currentNormal, ref q, out v);
                                                currentNormal = v;

        }
                                        }

                                        //now add the index
                                        int index = int.Parse(indexNormalPair[0]);
                                       
                                        int alreadyWrittenAt = index; //in case it is the first mesh
                                        if (!writtenVertices.TryGetValue(index, out alreadyWrittenAt)) //if we haven't  written it in this mesh pass, add it again unless it is the first one which we know has been written
                                        {
                                            //all vertices will be unique and have only one normal
                                            writtenVertices.Add(index, this.PositionCount);
                                            this.TriangleIndices.Add(this.PositionCount);
                                            this.Positions.Add(vertexList[index]);
                                            this.Normals.Add(currentNormal);
                                        }
                                        else //just add the index reference
                                        {
                                            this.TriangleIndices.Add(alreadyWrittenAt);
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

        static public XbimMeshGeometry3D MakeBoundingBox(XbimRect3D r3D, XbimMatrix3D transform)
        {
            XbimMeshGeometry3D mesh = new XbimMeshGeometry3D(8);
            XbimPoint3D p0 = transform.Transform(r3D.Location);
            XbimPoint3D p1 = p0;
            p1.X += r3D.SizeX;
            XbimPoint3D p2 = p1;
            p2.Z += r3D.SizeZ;
            XbimPoint3D p3 = p2;
            p3.X -= r3D.SizeX;
            XbimPoint3D p4 = p3;
            p4.Y += r3D.SizeY;
            XbimPoint3D p5 = p4;
            p5.Z -= r3D.SizeZ;
            XbimPoint3D p6 = p5;
            p6.X += r3D.SizeX;
            XbimPoint3D p7 = p6;
            p7.Z += r3D.SizeZ;


            mesh.Positions.Add(p0);
            mesh.Positions.Add(p1);
            mesh.Positions.Add(p2);
            mesh.Positions.Add(p3);
            mesh.Positions.Add(p4);
            mesh.Positions.Add(p5);
            mesh.Positions.Add(p6);
            mesh.Positions.Add(p7);

            mesh.TriangleIndices.Add(3);
            mesh.TriangleIndices.Add(0);
            mesh.TriangleIndices.Add(2);

            mesh.TriangleIndices.Add(0);
            mesh.TriangleIndices.Add(1);
            mesh.TriangleIndices.Add(2);

            mesh.TriangleIndices.Add(4);
            mesh.TriangleIndices.Add(5);
            mesh.TriangleIndices.Add(3);

            mesh.TriangleIndices.Add(5);
            mesh.TriangleIndices.Add(0);
            mesh.TriangleIndices.Add(3);

            mesh.TriangleIndices.Add(7);
            mesh.TriangleIndices.Add(6);
            mesh.TriangleIndices.Add(4);

            mesh.TriangleIndices.Add(6);
            mesh.TriangleIndices.Add(5);
            mesh.TriangleIndices.Add(4);

            mesh.TriangleIndices.Add(2);
            mesh.TriangleIndices.Add(1);
            mesh.TriangleIndices.Add(7);

            mesh.TriangleIndices.Add(1);
            mesh.TriangleIndices.Add(6);
            mesh.TriangleIndices.Add(7);

            mesh.TriangleIndices.Add(4);
            mesh.TriangleIndices.Add(3);
            mesh.TriangleIndices.Add(7);

            mesh.TriangleIndices.Add(3);
            mesh.TriangleIndices.Add(2);
            mesh.TriangleIndices.Add(7);

            mesh.TriangleIndices.Add(6);
            mesh.TriangleIndices.Add(1);
            mesh.TriangleIndices.Add(5);

            mesh.TriangleIndices.Add(1);
            mesh.TriangleIndices.Add(0);
            mesh.TriangleIndices.Add(5);

            return mesh;
        }

      

        #region standard calls

        private void Init()
        {
            indexOffset = (uint)Positions.Count;
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

        void IXbimTriangulatesToPositionsIndices.AddPosition(XbimPoint3D XbimPoint3D)
        {
            _points.Add(XbimPoint3D);
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
                    default:
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

        void IXbimTriangulatesToPositionsNormalsIndices.AddPosition(XbimPoint3D XbimPoint3D)
        {
            Positions.Add(XbimPoint3D);
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

        private int  Offset(uint index)
        {
            return (int)(index + indexOffset);
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
                    default:
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
            int startPosition = Positions.Count;
            XbimMeshFragment fragment = new XbimMeshFragment(startPosition, TriangleIndexCount, modelId);
            Positions.AddRange(toAdd.Positions);
            Normals.AddRange(toAdd.Normals);
            foreach (var idx in toAdd.TriangleIndices)
                 TriangleIndices.Add(idx+startPosition);
            fragment.EndPosition = PositionCount - 1;
            fragment.EndTriangleIndex = TriangleIndexCount - 1;
            fragment.EntityLabel = entityLabel;
            fragment.EntityTypeId = IfcMetaData.IfcTypeId(ifcType);
            meshes.Add(fragment);
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
            get { return meshes; }
            set
            {
                meshes = new XbimMeshFragmentCollection(value);
            }
        }

        /// <summary>
        /// Appends a geometry data object to the Mesh, returns false if the mesh would become too big and needs splitting
        /// </summary>
        /// <param name="geometryMeshData"></param>
        public bool Add(XbimGeometryData geometryMeshData, short modelId)
        {
            XbimMatrix3D transform = XbimMatrix3D.FromArray(geometryMeshData.DataArray2);
            if (geometryMeshData.GeometryType == XbimGeometryType.TriangulatedMesh)
            {
                XbimTriangulatedModelStream strm = new XbimTriangulatedModelStream(geometryMeshData.ShapeData);
                XbimMeshFragment fragment = strm.BuildWithNormals(this, transform);
                if (fragment.EntityLabel==-1) //nothing was added due to size being exceeded
                    return false;
                else //added ok
                {
                    fragment.EntityLabel = geometryMeshData.IfcProductLabel;
                    fragment.EntityTypeId = geometryMeshData.IfcTypeId;
                    meshes.Add(fragment);
                }
            }
            else if (geometryMeshData.GeometryType == XbimGeometryType.BoundingBox)
            {
                XbimRect3D r3d = XbimRect3D.FromArray(geometryMeshData.ShapeData);
                this.Add(XbimMeshGeometry3D.MakeBoundingBox(r3d, transform), geometryMeshData.IfcProductLabel, IfcMetaData.GetType(geometryMeshData.IfcTypeId), modelId);
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
            if (meshes.Any()) //if no meshes nothing to move
            {
                toMesh.BeginUpdate();
                toMesh.Positions = this.Positions; this.Positions.Clear();
                toMesh.Normals = this.Normals; this.Normals.Clear();
                toMesh.TriangleIndices = this.TriangleIndices; this.TriangleIndices.Clear();
                toMesh.Meshes = this.Meshes; this.meshes.Clear();
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
            XbimMeshGeometry3D m3d = new XbimMeshGeometry3D();
            for (int i = frag.StartPosition; i <= frag.EndPosition; i++)
            {
                m3d.Positions.Add(this.Positions[i]);
                m3d.Normals.Add(this.Normals[i]);
            }
            for (int i = frag.StartTriangleIndex; i <= frag.EndTriangleIndex; i++)
            {
                m3d.TriangleIndices.Add(this.TriangleIndices[i] - frag.StartPosition);
            }
            return m3d;
        }

        /// <summary>
        /// Adds a geometry mesh to this, includes all mesh fragments
        /// </summary>
        /// <param name="geom"></param>
        public void Add(IXbimMeshGeometry3D geom)
        {
            if (geom.Positions.Any()) //if no positions nothing to add
            {
                this.BeginUpdate();
                int startPos = Positions.Count;
                int startIndices = TriangleIndices.Count;
                Positions.AddRange(geom.Positions);
                Normals.AddRange(geom.Normals);
                foreach (var indices in geom.TriangleIndices)
                    TriangleIndices.Add(indices + startPos);
                foreach (var fragment in geom.Meshes)
                {
                    fragment.Offset(startPos, startIndices);
                    Meshes.Add(fragment);
                }

                this.EndUpdate();
            }
        }

        public byte[] ToByteArray()
        {
            //calculate the indices count
            int pointCount = 0;
            foreach (var mesh in meshes)
                pointCount += mesh.EndTriangleIndex - mesh.StartTriangleIndex + 1;
            byte[] bytes = new byte[pointCount * ((6 * sizeof(float)) + sizeof(int))]; //max size of data stream
            MemoryStream ms = new MemoryStream(bytes);
            BinaryWriter bw = new BinaryWriter(ms);
            foreach (var mesh in meshes)
            {
                int label = mesh.EntityLabel;
                label =  ((label & 0xff) << 24) + ((label & 0xff00) << 8) + ((label & 0xff0000) >> 8) + ((label >> 24) & 0xff);
                for (int i = mesh.StartTriangleIndex; i <= mesh.EndTriangleIndex; i++)
                {
                    XbimPoint3D pt = Positions[TriangleIndices[i]];
                    XbimVector3D n = Normals[TriangleIndices[i]];
                    bw.Write(pt.X); bw.Write(pt.Y); bw.Write(pt.Z);
                    bw.Write(n.X); bw.Write(n.Y); bw.Write(n.Z);
                    bw.Write(label); 
                }
            }
            return bytes;
        }


        public XbimRect3D GetBounds()
        {
            bool first = true;
            XbimRect3D boundingBox = XbimRect3D.Empty;
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
        public XbimMeshFragment Add(IXbimGeometryModel geometryModel, IfcProduct product, XbimMatrix3D transform, double? deflection = null, short modelId=0)
        {
            return geometryModel.MeshTo(this,product,transform,deflection??product.ModelOf.ModelFactors.DeflectionTolerance);
        }



        public void Add(string mesh, Type productType, int productLabel, int geometryLabel, XbimMatrix3D? transform, short modelId)
        {
            Add(mesh, IfcMetaData.IfcTypeId(productType), productLabel, geometryLabel, transform, modelId);

        }

        public void Add(string mesh, short productTypeId, int productLabel, int geometryLabel, XbimMatrix3D? transform, short modelId)
        {
            lock (meshLock)
            {
                XbimMeshFragment frag = new XbimMeshFragment(PositionCount, TriangleIndexCount, productTypeId, productLabel, geometryLabel, modelId);
                Read(mesh, transform);
                frag.EndPosition = PositionCount - 1;
                frag.EndTriangleIndex = TriangleIndexCount - 1;
                meshes.Add(frag);
            }

        }

       
      
       
    }
}