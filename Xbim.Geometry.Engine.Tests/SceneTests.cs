using System;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.IO;
using Xbim.ModelGeometry.Scene;
using XbimGeometry.Interfaces;

namespace GeometryTests
{
    [TestClass]
   
    public class SceneTests
    {
        /// <summary>
        /// Reads and writes the geometry of an Ifc file to WexBIM format
        /// </summary>
        [TestMethod]
        public void ReadAndWriteWexBimFile()
        {
            using (var m = new XbimModel())
            {
                m.CreateFrom("SolidTestFiles\\19 - TwoProxy.ifc", null, null, true, true);
                var m3D = new Xbim3DModelContext(m);
                m3D.CreateContext(XbimGeometryType.PolyhedronBinary);
                using (var bw = new BinaryWriter(new FileStream("test.wexBIM", FileMode.Create)))
                {
                    m3D.Write(bw);
                    bw.Close();
                }
                using (var fs = new FileStream(@"test.wexBIM", FileMode.Open, FileAccess.Read))
                {
                    using (var br = new BinaryReader(fs))
                    {
                        var magicNumber = br.ReadInt32();
                        var version = br.ReadByte();
                        var shapeCount = br.ReadInt32();
                        var vertexCount = br.ReadInt32();
                        var triangleCount = br.ReadInt32();
                        var matrixCount = br.ReadInt32();
                        var productCount = br.ReadInt32();
                        var styleCount = br.ReadInt32();
                        var meter =  br.ReadSingle();
                        Assert.IsTrue(meter>0);
                        var regionCount = br.ReadInt16();
                        for (int i = 0; i < regionCount; i++)
                        {
                            var population = br.ReadInt32();
                            var centreX = br.ReadSingle();
                            var centreY = br.ReadSingle();
                            var centreZ = br.ReadSingle();
                            var boundsBytes = br.ReadBytes(6 * sizeof(float));
                            var modelBounds = XbimRect3D.FromArray(boundsBytes);
                        }
                        
                        for (int i = 0; i < styleCount; i++)
                        {
                            var styleId = br.ReadInt32();
                            var red = br.ReadSingle();
                            var green = br.ReadSingle();
                            var blue = br.ReadSingle();
                            var alpha = br.ReadSingle();
                        }
                        for (int i = 0; i < productCount ; i++)
                        {
                            var productLabel = br.ReadInt32();
                            var productType = br.ReadInt16();
                            var boxBytes = br.ReadBytes(6 * sizeof(float));
                            XbimRect3D bb = XbimRect3D.FromArray(boxBytes);
                        }
                        for (int i = 0; i < shapeCount; i++)
                        {
                            var shapeRepetition = br.ReadInt32();
                            Assert.IsTrue(shapeRepetition > 0);
                            if (shapeRepetition > 1)
                            {
                                for (int j = 0; j < shapeRepetition; j++)
                                {
                                    var ifcProductLabel = br.ReadInt32();
                                    var instanceTypeId = br.ReadInt16();
                                    var instanceLabel = br.ReadInt32();
                                    var styleId = br.ReadInt32();
                                    var transform = XbimMatrix3D.FromArray(br.ReadBytes(sizeof(float) * 16));
                                }
                                var triangulation = br.ReadShapeTriangulation();
                            }
                            else if (shapeRepetition == 1)
                            {
                                var ifcProductLabel = br.ReadInt32();
                                var instanceTypeId = br.ReadInt16();
                                var instanceLabel = br.ReadInt32();
                                var styleId = br.ReadInt32();
                                var triangulation = br.ReadShapeTriangulation();
                            }  
                        }
                    } 
                }
                
            }
        }
    }
}
