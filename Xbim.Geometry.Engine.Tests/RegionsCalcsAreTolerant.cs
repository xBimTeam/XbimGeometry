using System.IO;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [TestClass]
    public class RegionsCalcsAreTolerantTests
    {
        // this test is requires a private file.
        // it will just pass if the file is not available.
        //
        [TestMethod]
        public void Exec()
        {
            var dir = @"C:\Users\sgmk2\OneDrive\CloudIfcArchive\VP";
            if (!Directory.Exists(dir))
                return;

            var files = Directory.EnumerateFiles(dir);

            foreach (var file in files)
            {
                ProcessIFC(file);
            }
        }

        private void ProcessIFC(string file)
        {
            var f = new FileInfo(file);
            if (f.Extension.Contains("_"))
                return;

            using (IfcStore model = IfcStore.Open(file))
            {
                var m3D = new Xbim3DModelContext(model);
                m3D.CreateContext();

                var name = Path.GetTempFileName();
                name = Path.ChangeExtension(name, "xbim");
                model.SaveAs(name);
                // var shapes = m3D.ShapeGeometries();
            }
        }
    }
}
