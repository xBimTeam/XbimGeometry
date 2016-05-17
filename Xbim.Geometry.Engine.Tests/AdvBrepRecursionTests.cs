using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene.Extensions;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"Ifc4TestFiles\", "Ifc4TestFiles")]
    [TestClass]
    public class AdvBrepRecursionTests
    {
        [TestMethod]
        public void CreateAdvancedBrep()
        {
            var files = Directory.GetFiles(@"Ifc4TestFiles\NBSAdvancedBreps", "*.ifc");
            foreach (var file in files)
            {
                using (var store = IfcStore.Open(file))
                {
                    var products = store.Instances.OfType<IIfcProduct>();

                    foreach (var product in products)
                    {
                        var reps = GetGeometryRepresentationsForProduct(product);
                        var geomStore = store.GeometryStore;
                        var engine = new XbimGeometryEngine();
                        foreach (var rep in reps)
                        {
                            Debugger.Break();
                            var geometryObjectSet = engine.Create(rep);
                        }
                    }
                }
            }
        }

        private static List<IIfcGeometricRepresentationItem> GetGeometryRepresentationsForProduct(IIfcProduct product)
        {
            if (product == null) throw new Exception("No component found");
            if (product.Representation == null) throw new Exception("No representation found");
            if (product.Representation.Representations == null) throw new Exception("No representations found");

            var representation = product.Representation.Representations.FirstOrDefault(p => p.IsBodyRepresentation());
            if (representation == null) throw new Exception("No representation items found");

            var geomReps = representation.Items.ToList();
            var reps = new List<IIfcGeometricRepresentationItem>();

            foreach (var geomRep in geomReps)
            {
                if (geomRep.GetType().Name == "IfcMappedItem")
                {
                    var mappedItem = geomRep as IIfcMappedItem;
                    if (mappedItem != null)
                        reps.AddRange(mappedItem.MappingSource.MappedRepresentation.Items.Select(rep => rep as IIfcGeometricRepresentationItem));
                }
                else
                {
                    reps.Add(geomRep as IIfcGeometricRepresentationItem);
                }
            }
            return reps;
        }
    }
}
