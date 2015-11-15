using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Xbim.Common.Geometry;
using Xbim.IO;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// A class to handle the conversion to SceneJS format
    /// </summary>
    public class XbimSceneJS
    {
        private readonly Xbim3DModelContext _context;
        private HashSet<int> _maps = new HashSet<int>();

        public XbimSceneJS(Xbim3DModelContext context)
        {
            _context = context;
        }

        public void WriteScene(JsonWriter writer)
        {
            WriteCamera(writer);
        }

        private void WriteCamera(JsonWriter writer)
        {
            //writer.WriteStartObject(); //the main scene
            //writer.WritePropertyName("nodes");
            //writer.WriteStartArray();
            writer.WriteStartObject(); //the camera node
            writer.WritePropertyName("type"); writer.WriteValue("cameras/orbit"); //main camera node
            writer.WritePropertyName("id"); writer.WriteValue("camera");
            writer.WritePropertyName("yaw"); writer.WriteValue(30);
            writer.WritePropertyName("pitch"); writer.WriteValue(-30);
            writer.WritePropertyName("zoom"); writer.WriteValue(50);
            writer.WritePropertyName("zoomSensitivity"); writer.WriteValue(1);
            writer.WritePropertyName("nodes");
            writer.WriteStartArray();
         //   WritePerspectiveCamera(writer);

            WriteLibrary(writer);
            WriteInstances(writer);

          

            writer.WriteEndArray();
            writer.WriteEndObject(); //end of orbit camera

            //writer.WriteEndArray();
            //writer.WriteEndObject(); //end of nodes
        }

        private static void WritePerspectiveCamera(JsonWriter writer)
        {
            writer.WriteStartObject(); //perspective camera
            writer.WritePropertyName("type"); writer.WriteValue("camera"); //main camera node
            writer.WritePropertyName("id"); writer.WriteValue("perspectiveCamera");
            writer.WritePropertyName("optics");

            writer.WriteStartObject(); //start optics
            writer.WritePropertyName("type"); writer.WriteValue("perspective");
            writer.WritePropertyName("fovy"); writer.WriteValue(45.0);
            writer.WritePropertyName("aspect"); writer.WriteValue(1.0);
            writer.WritePropertyName("near"); writer.WriteValue(0.1);
            writer.WritePropertyName("far"); writer.WriteValue(5000.0);
            writer.WriteEndObject(); //end optics


            writer.WritePropertyName("nodes");
            writer.WriteStartArray();
            //write next node
            writer.WriteEndArray();
            writer.WriteEndObject(); //end of perspective camera
        }

        /// <summary>
        /// calculates the tansform to convert models to metres and centre on the most populated region, includes reference models
        /// </summary>
        /// <returns></returns>
        private XbimMatrix3D GetGlobalModelTransform()
        {
            XbimRegion largest = _context.GetLargestRegion();
            
            XbimRect3D bb = XbimRect3D.Empty;
            if (largest != null) bb = new XbimRect3D(largest.Centre, largest.Centre);
            throw new NotImplementedException();//need to fix this
            //foreach (var refModel in _context.Model.ReferencedModels)
            //{
            //    XbimRegion r;
            //    Xbim3DModelContext refContext = new Xbim3DModelContext(refModel.Model);
            //    r = refContext.GetLargestRegion();
            //    if (r != null)
            //    {
            //        if (bb.IsEmpty)
            //            bb = new XbimRect3D(r.Centre, r.Centre);
            //        else
            //            bb.Union(r.Centre);
            //    }
            //}
            XbimPoint3D p = bb.Centroid();
            var modelTranslation = new XbimVector3D(-p.X, -p.Y, -p.Z);
            double metreFactor = 1.0 / _context.Model.ModelFactors.OneMetre;
            return XbimMatrix3D.CreateTranslation(modelTranslation) * XbimMatrix3D.CreateScale(metreFactor);
        }

        //private void WriteInstances(JsonWriter writer)
        //{
        //    //write a flag node for all pickable instances
        //    writer.WriteStartObject();
        //    writer.WritePropertyName("type"); //instance node
        //    writer.WriteValue("matrix");
        //    writer.WritePropertyName("id");
        //    writer.WriteValue("instances");
        //    writer.WritePropertyName("elements");
        //   // WriteMatrix(writer, GetGlobalModelTransform());
        //    WriteMatrix(writer, XbimMatrix3D.Identity);
        //    writer.WritePropertyName("nodes");
        //    writer.WriteStartArray();

        //    WriteGeometry(writer);

        //    writer.WriteEndArray();
        //    writer.WriteEndObject();
        //}
        private void WriteInstances(JsonWriter writer)
        {
            XbimMatrix3D globalTrans = GetGlobalModelTransform();
            //write out the material nodes and then the instances for each material
         //  Dictionary<int, XbimTexture> styles = _context.SurfaceStyles().ToDictionary(s => s.DefinedObjectId);

            //write all pickable instances
            writer.WriteStartObject();
            foreach (var shapeGeom in _context.ShapeGeometries())
            {
                writer.WriteRaw(shapeGeom.ShapeData); //the vertex geometry
                writer.WritePropertyName("Shapes"); //instance nodes
                writer.WriteStartArray();
                foreach (var shapeInstance in _context.ShapeInstancesOf(shapeGeom))
                {
                    writer.WriteStartObject();
                    writer.WritePropertyName("id"); writer.WriteValue(shapeInstance.InstanceLabel);
                    writer.WritePropertyName("pid"); writer.WriteValue(shapeInstance.IfcProductLabel);
                    writer.WritePropertyName("tr"); WriteMatrix(writer, shapeInstance.Transformation);
                    writer.WritePropertyName("sid"); writer.WriteValue(shapeInstance.StyleLabel);
                    writer.WriteEndObject();
                }
                writer.WriteEndArray();
            }
            writer.WriteEndObject();
        }
        private void WriteGeometry(JsonWriter writer)
        {
            XbimMatrix3D globalTrans =  GetGlobalModelTransform();
            //write out the material nodes and then the instances for each material
            Dictionary<int, XbimTexture> styles = _context.SurfaceStyles().ToDictionary(s => s.DefinedObjectId);
            foreach (var instanceGroup in _context.ShapeInstancesGroupByStyle())
            {
                if (!instanceGroup.Any()) continue; //skip emmpty instances;
                int styleId = instanceGroup.Key;
                XbimTexture style = styles[styleId];
                writer.WriteStartObject(); //Material node
                {
                    writer.WritePropertyName("type"); writer.WriteValue("material");
                    writer.WritePropertyName("id"); writer.WriteValue("M" + styleId);
                    writer.WritePropertyName("color");
                    writer.WriteStartObject(); //begin color
                    {
                        writer.WritePropertyName("r"); writer.WriteValue(style.ColourMap[0].Red);
                        writer.WritePropertyName("g"); writer.WriteValue(style.ColourMap[0].Green);
                        writer.WritePropertyName("b"); writer.WriteValue(style.ColourMap[0].Blue);
                    } writer.WriteEndObject(); //end color
                    writer.WritePropertyName("alpha"); writer.WriteValue(style.ColourMap[0].Alpha);
                    //all instances of this style
                    writer.WritePropertyName("nodes"); //beginning of the instance nodes
                    writer.WriteStartArray();
                    foreach (var shapeInstance in instanceGroup)
                    {
                        writer.WriteStartObject(); //start of instance transform
                        {

                            if (_maps.Contains(shapeInstance.ShapeGeometryLabel)) //it is a reference
                            {
                                //write the transform
                                writer.WritePropertyName("type"); //geometry node
                                writer.WriteValue("matrix");
                                writer.WritePropertyName("id");
                                writer.WriteValue("I" + shapeInstance.InstanceLabel);
                                writer.WritePropertyName("elements");
                                XbimMatrix3D m = XbimMatrix3D.Multiply(shapeInstance.Transformation, globalTrans);
                                WriteMatrix(writer, m);
                                writer.WritePropertyName("nodes"); //beginning of the instance nodes
                                writer.WriteStartArray();
                                //write the map
                                writer.WriteStartObject(); //start of map 
                                {
                                    writer.WritePropertyName("type"); //geometry node
                                    writer.WriteValue("geometry");
                                    writer.WritePropertyName("coreId");
                                    writer.WriteValue("L" + shapeInstance.ShapeGeometryLabel);
                                } writer.WriteEndObject(); //end of map 
                                writer.WriteEndArray(); //end of instance tranform nodes
                            }
                            else //write the actual geometry
                            {
                                writer.WritePropertyName("type"); //geometry node
                                writer.WriteValue("geometry");
                                writer.WritePropertyName("primitive"); //primitive: "
                                writer.WriteValue("triangles");
                                writer.WritePropertyName("id");
                                writer.WriteValue("I" + shapeInstance.InstanceLabel);
                                WriteShapeGeometry(writer, _context.ShapeGeometry(shapeInstance), XbimMatrix3D.Multiply(shapeInstance.Transformation, globalTrans));
                            }

                        } writer.WriteEndObject(); //end of instance transform
                    }
                    writer.WriteEndArray(); //end of instance transform nodes

                } writer.WriteEndObject(); // end of the material node
            }
        }

        private void WriteMatrix(JsonWriter writer, XbimMatrix3D m)
        {
            writer.WriteStartArray(); //matrix elements
            writer.WriteValue(m.M11); writer.WriteValue(m.M12); writer.WriteValue(m.M13); writer.WriteValue(m.M14);
            writer.WriteValue(m.M21); writer.WriteValue(m.M22); writer.WriteValue(m.M23); writer.WriteValue(m.M24);
            writer.WriteValue(m.M31); writer.WriteValue(m.M32); writer.WriteValue(m.M33); writer.WriteValue(m.M34);
            writer.WriteValue(m.OffsetX); writer.WriteValue(m.OffsetY); writer.WriteValue(m.OffsetZ); writer.WriteValue(m.M44);
            writer.WriteEndArray(); //end of matrix elements
        }

        private void WriteLibrary(JsonWriter writer)
        {
            _maps = new HashSet<int>();
            writer.WriteStartObject();
            {
                //first write out the library nodes
                writer.WritePropertyName("type"); //library node
                writer.WriteValue("library");
                writer.WritePropertyName("nodes");
                writer.WriteStartArray();
                foreach (var shapeGeom in _context.ShapeGeometries().Where(sg => sg.ReferenceCount > 1))
                {
                    _maps.Add(shapeGeom.ShapeLabel); //keep a record of all geometries that are maps
                    writer.WriteStartObject(); //a geometry object
                    {
                        writer.WritePropertyName("type"); //geometry node
                        writer.WriteValue("geometry");
                        writer.WritePropertyName("primitive"); //primitive: "
                        writer.WriteValue("triangles");
                        writer.WritePropertyName("coreId");
                        writer.WriteValue("L" + shapeGeom.ShapeLabel);
                        WriteShapeGeometry(writer, shapeGeom);
                    }
                    writer.WriteEndObject();
                }
                writer.WriteEndArray();
            }
            writer.WriteEndObject(); //library node 
        }

        private void WriteShapeGeometry(JsonWriter writer, XbimShapeGeometry geom, XbimMatrix3D? transform = null)
        {
            XbimMeshGeometry3D mesh = new XbimMeshGeometry3D();
            mesh.Read(geom.ShapeData, transform);
            writer.WritePropertyName("positions");
            writer.WriteStartArray();
            foreach (var point in mesh.Positions)
            {
                writer.WriteValue(Math.Round(point.X,4)); //we are converting to metres so this is effectively .1mm
                writer.WriteValue(Math.Round(point.Y,4));
                writer.WriteValue(Math.Round(point.Z,4));
            }
            writer.WriteEndArray();
            writer.WritePropertyName("indices");
            writer.WriteStartArray();
            foreach (var idx in mesh.TriangleIndices)
            {
                writer.WriteValue(idx);
            }
            writer.WriteEndArray();
            writer.WritePropertyName("normals");
            writer.WriteStartArray();
            foreach (var norm in mesh.Normals)
            {
                writer.WriteValue(norm.X);
                writer.WriteValue(norm.Y);
                writer.WriteValue(norm.Z);
            }
            writer.WriteEndArray();
        }
    }
}
