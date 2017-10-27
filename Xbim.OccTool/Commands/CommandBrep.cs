using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;

namespace Xbim.OccTool.Commands
{
    class CommandBrep : Command
    {
        internal static readonly ILogger Log = LoggerFactory.GetLogger();

        static new string Name = "Brep";
        internal static new string Syntax = "brep EntityLabel... FileName";

        internal override string GetName()
        {
            return Name;
        }

        internal override string GetSyntax()
        {
            return Syntax;
        }

        [Flags]
        internal enum validOptions
        {
            None, 
            /// <summary>
            /// ProgressiveFileNames
            /// </summary>
            PFN = 1
        }

        internal override int Process(string[] args, int index)
        {
            var options = validOptions.None;
            while (index < args.Length)
            {
                var nm = args[index];
                if (nm == "PFN")
                {
                    options |= validOptions.PFN;
                    index++;
                }
                else
                {
                    break;
                }
            }

            // parse entity list
            List<int> entities = new List<int>();
            while (index < args.Length)
            {
                int el;
                if (!int.TryParse(args[index], out el))
                    break;
                entities.Add(el);
                index++;
            }
            if (!entities.Any())
            {
                Log.Error($"Not enough parameters: No entities specified.");
                return 100;
            }
            
            // parse filename
            FileInfo IfcFile = null;
            if (index < args.Length)
            {
                IfcFile = new FileInfo(args[index]);
                if (!IfcFile.Exists)
                {
                    Log.Error($"File {IfcFile.FullName} not found.");
                    return 100;
                }
            }

            if (IfcFile == null)
            {
                Log.Error($"Not enough parameters: No file specified.");
                return 100;
            }

            // we can work on the file
            var geomEngine = new XbimGeometryEngine();
            using (var m = IfcStore.Open(IfcFile.FullName))
            {
                DirectoryInfo dOut = new DirectoryInfo(Path.Combine(
                    IfcFile.Directory.FullName,
                    Path.GetFileNameWithoutExtension(IfcFile.FullName)
                    ));
                int iProgressive = 0;
                foreach (var entityLabel in entities)
                {
                    iProgressive++;
                    var ent = m.Instances[entityLabel];
                    if (ent == null)
                    {

                        Log.Error($"Entity #{entityLabel} not found in the model.");
                        continue;
                    }
                    var geomEntity = ent as IIfcGeometricRepresentationItem;
                    if (geomEntity == null)
                    {
                        Log.Error($"Entity #{entityLabel} is not a valid IIfcGeometricRepresentationItem.");
                        continue;
                    }

                    var geometryObject = geomEngine.Create(geomEntity);
                    if (geometryObject == null)
                    {
                        Log.Error($"Geometry object creation failed for #{entityLabel}.");
                        continue;
                    }

                    var brep = geomEngine.ToBrep(geometryObject);
                    if (string.IsNullOrEmpty(brep))
                    {
                        Log.Error($"Empty BREP for #{entityLabel}.");
                        continue;
                    }

                    var localFileName = entityLabel.ToString();
                    if (options.HasFlag(validOptions.PFN))
                    {
                        localFileName = iProgressive.ToString();
                    }

                    var wFileName = Path.Combine(dOut.FullName , $"{localFileName}.brep");
                    dOut.Create();
                    var wFileInfo = new FileInfo(wFileName);
                    using (var wFileStream = wFileInfo.CreateText())
                    {
                        wFileStream.WriteLine("DBRep_DrawableShape"); // required in brep file for occ
                        wFileStream.Write(brep);
                        wFileStream.Close();
                        Console.WriteLine($"Element #{entityLabel} saved.");
                    }
                }
            }
            return 100;
        }
    }
}