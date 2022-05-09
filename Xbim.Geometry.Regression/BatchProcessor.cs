using Microsoft.Extensions.Logging;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;

namespace XbimRegression
{
    /// <summary>
    /// Class to process a folder of IFC files, capturing key metrics about their conversion
    /// </summary>
    public class BatchProcessor
    {

        readonly Params _params;

        public BatchProcessor(Params arguments)
        {
            _params = arguments;
        }

        public Params Params
        {
            get { return _params; }
        }

        public void Run()
        {
            FileInfo f = new FileInfo(Params.ResultsFile);
            Console.WriteLine($"Reporting to \"{f.FullName}\"");

            using var writer = new StreamWriter(Params.ResultsFile);
            writer.WriteLine(ProcessResult.CsvHeader);
            // ParallelOptions opts = new ParallelOptions() { MaxDegreeOfParallelism = 12 };

            // Parallel.ForEach<FileInfo>(toProcess, opts, file =>
            foreach (var file in Params.FilesToProcess)
            {
                //set up a  log file for this file run                 
                var logFile = Path.ChangeExtension(file.FullName, "log");
                ProcessResult result;
                using (var loggerFactory = new LoggerFactory())
                {
                    XbimLogging.LoggerFactory = loggerFactory;
                    loggerFactory.AddConsole(LogLevel.Error);
                    loggerFactory.AddProvider(new NReco.Logging.File.FileLoggerProvider(logFile, false)
                    {
                        FormatLogEntry = (msg) =>
                        {
                            var sb = new System.Text.StringBuilder();
                            StringWriter sw = new StringWriter(sb);
                            var jsonWriter = new Newtonsoft.Json.JsonTextWriter(sw);
                            jsonWriter.WriteStartArray();
                            jsonWriter.WriteValue(DateTime.Now.ToString("o"));
                            jsonWriter.WriteValue(msg.LogLevel.ToString());
                            jsonWriter.WriteValue(msg.EventId.Id);
                            jsonWriter.WriteValue(msg.Message);
                            jsonWriter.WriteValue(msg.Exception?.ToString());
                            jsonWriter.WriteEndArray();
                            return sb.ToString();
                        }
                    });
                    var logger = loggerFactory.CreateLogger<BatchProcessor>();
                    Console.WriteLine($"Processing {file}");
                    result = ProcessFile(file.FullName, writer, logger);
                }
                if (result == null)
                {
                    continue;
                }

                XbimLogging.LoggerFactory = null; // uses a default loggerFactory

                var txt = File.ReadAllText(logFile);
                if (string.IsNullOrEmpty(txt))
                {
                    File.Delete(logFile);
                    result.Errors = 0;
                    result.Warnings = 0;
                    result.Information = 0;
                }
                else
                {

                    var tokens = txt.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                    result.Errors = tokens.Count(t => t == "\"Error\"");
                    result.Warnings = tokens.Count(t => t == "\"Warning\"");
                    result.Information = Math.Max(0, tokens.Count(t => t == "\"Information\"") - 2); //we always get 2
                }
                if (result != null && !result.Failed)
                {
                    Console.WriteLine($"Processed {file} : {result.Errors} Errors, {result.Warnings} Warnings, {result.Information} Informational in {result.TotalTime}ms. {result.Entities} IFC Elements & {result.GeometryEntries} Geometry Nodes.");
                }
                else
                {
                    Console.WriteLine("Processing failed for {0} after {1}ms.", file, result.TotalTime);
                }
                result.FileName = file.Name;
                writer.WriteLine(result.ToCsv());
                writer.Flush();
            }

            writer.Close();

            Console.WriteLine("Finished. Press Enter to continue...");

            Console.ReadLine();
        }

        string lastState = "";
        int lastPerc = -1;
        bool stateIsComplete = false;

        private void InitProgress()
        {
            lastState = "";
            lastPerc = -1;
            stateIsComplete = true;
        }


        private void progressReport(int percentProgress, object userState)
        {
            if (percentProgress > 100)
            {
                stateIsComplete = true;
                Console.WriteLine("");
            }
            if (userState.ToString() != lastState)
            {
                lastState = userState.ToString();
                if (!stateIsComplete)
                    Console.WriteLine("");
                Console.Write($"{lastState}");
                stateIsComplete = false;
            }
            if (percentProgress < 0 || percentProgress > 100)
                return;
            if (lastPerc == percentProgress)
                return;
            lastPerc = percentProgress;
            Console.Write($" {percentProgress}%");
            stateIsComplete = false;
        }

        private ProcessResult ProcessFile(string ifcFile, StreamWriter writer, ILogger<BatchProcessor> logger)
        {
            RemoveFiles(ifcFile);
            // using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                var result = new ProcessResult() { Errors = -1 };
                var watch = new Stopwatch();
                try
                {
                    ReportProgressDelegate progress = null;
                    if (_params.ReportProgress)
                    {
                        InitProgress();
                        progress = progressReport;
                    }
                    watch.Start();
                    using (var model = ParseModelFile(ifcFile, Params.Caching, logger, progress))
                    {
                        if (model == null)
                            return null;
                        var parseTime = watch.ElapsedMilliseconds;
                        var xbimFilename = BuildFileName(ifcFile, ".xbim");
                        var context = new Xbim3DModelContext(model, logger: logger);
                        if (_params.MaxThreads > 0)
                            context.MaxThreads = _params.MaxThreads;
                        // context.CustomMeshingBehaviour = CustomMeshingBehaviour;
                        if (_params.WriteBreps == null)
                        {
                            context.CreateContext(progress);
                            //}
                            var geomTime = watch.ElapsedMilliseconds - parseTime;
                            //XbimSceneBuilder sb = new XbimSceneBuilder();
                            //string xbimSceneName = BuildFileName(ifcFile, ".xbimScene");
                            //sb.BuildGlobalScene(model, xbimSceneName);
                            // sceneTime = watch.ElapsedMilliseconds - geomTime;
                            var header = model.Header;
                            watch.Stop();
                            var ohs = model.Instances.OfType<IIfcOwnerHistory>().FirstOrDefault();
                            using (var geomReader = model.GeometryStore.BeginRead())
                            {
                                result = new ProcessResult
                                {
                                    ParseDuration = parseTime,
                                    GeometryDuration = geomTime,
                                    // SceneDuration = sceneTime,
                                    FileName = ifcFile.Remove(0, Params.TestFileRoot.Length).TrimStart('\\'),
                                    Entities = model.Instances.Count,
                                    IfcSchema = header.FileSchema.Schemas.FirstOrDefault(),
                                    IfcDescription =
                                        string.Format("{0}, {1}", header.FileDescription.Description.FirstOrDefault(),
                                            header.FileDescription.ImplementationLevel),
                                    GeometryEntries = geomReader.ShapeInstances.Count(),
                                    IfcLength = ReadFileLength(ifcFile),
                                    XbimLength = ReadFileLength(xbimFilename),
                                    SceneLength = 0,
                                    IfcProductEntries = model.Instances.OfType<IIfcProduct>().Count(),
                                    IfcSolidGeometries = model.Instances.OfType<IIfcSolidModel>().Count(),
                                    IfcMappedGeometries = model.Instances.OfType<IIfcMappedItem>().Count(),
                                    BooleanGeometries = model.Instances.OfType<IIfcBooleanResult>().Count(),
                                    BReps = model.Instances.OfType<IIfcFaceBasedSurfaceModel>().Count() +
                                            model.Instances.OfType<IIfcShellBasedSurfaceModel>().Count() + model.Instances
                                                .OfType<IIfcManifoldSolidBrep>().Count(),
                                    Application = ohs == null ? "Unknown" : ohs.OwningApplication?.ApplicationFullName.ToString()
                                };
                            }
                        }

                        // Option to save breps of encountered classes by type or entityLabel for debugging purposes

                        if (_params.WriteBreps != null)
                        {
                            var path = Path.Combine(
                                    Path.GetDirectoryName(ifcFile),
                                    Path.GetFileName(ifcFile) + ".brep.unclassified");
                            IXbimGeometryEngine engine = new XbimGeometryEngine();
                            if (!Directory.Exists(path))
                                Directory.CreateDirectory(path);
                            IfcStore s = model as IfcStore;
                            if (s != null)
                            {
                                var ents = new List<IPersistEntity>();

                                // ADD Individual entities to extract brep here
                                // 
                                if (_params.WriteBreps.Any())
                                {
                                    foreach (var item in _params.WriteBreps)
                                    {
                                        ents.Add(s.Instances[item]);
                                    }
                                }
                                else
                                {
                                    // otherwise export the default types
                                    //
                                    var exportBrepByType = new string[]
                                    {
                                    "IfcFacetedBrep",
								    // IIfcGeometricRepresentationItem
								    "IfcCsgSolid",
                                    "IfcExtrudedAreaSolid",
                                    "IfcExtrudedAreaSolidTapered",
                                    "IfcFixedReferenceSweptAreaSolid",
                                    "IfcRevolvedAreaSolid",
                                    "IfcRevolvedAreaSolidTapered",
                                    "IfcSurfaceCurveSweptAreaSolid",
                                    "IfcSectionedSolidHorizontal",
                                    "IfcSweptDiskSolid",
                                    "IfcSweptDiskSolidPolygonal",
                                    "IfcBooleanResult",
                                    "IfcBooleanClippingResult",
								    // composing objects
								    "IfcConnectedFaceSet"
                                    };
                                    foreach (var type in exportBrepByType)
                                    {
                                        ents.AddRange(s.Instances.OfType(type, false));
                                    }
                                }
                                foreach (var ent in ents)
                                {
                                    try
                                    {
                                        Xbim.Common.Geometry.IXbimGeometryObject created = null;
                                        if (ent is IIfcGeometricRepresentationItem igri)
                                            created = engine.Create(igri);
                                        if (ent is IIfcConnectedFaceSet icfs)
                                            created = engine.CreateShell(icfs);
                                        // IIfcConnectedFaceSet
                                        if (created != null)
                                        {
                                            var brep = engine.ToBrep(created);
                                            var brepFileName = Path.Combine(path, $"{ent.EntityLabel}.{ent.GetType().Name}.brep");
                                            using (var tw = File.CreateText(brepFileName))
                                            {
                                                tw.WriteLine("DBRep_DrawableShape");
                                                tw.WriteLine(brep);
                                            }
                                        }
                                    }
                                    catch (Exception ex)
                                    {
                                        Console.WriteLine($"Error writing brep {ent.EntityLabel}: {ex.Message}");
                                    }
                                }
                            }
                        }

                        if (_params.Caching)
                        {
                            if (_params.ReportProgress)
                                Console.WriteLine($"Writing cache file '{xbimFilename}'");
                            IfcStore s = ((IfcStore)model);
                            if (s != null)
                            {
                                s.SaveAs(xbimFilename, Xbim.IO.StorageType.Xbim);
                                s.Close();
                            }
                        }
                    }
                }

                catch (Exception ex)
                {
                    logger.LogError(string.Format("Problem converting file: {0}", ifcFile), ex);
                    result.Failed = true;
                    result.GeometryDuration = watch.ElapsedMilliseconds;
                }

                return result;
            }
        }



        private Xbim3DModelContext.MeshingBehaviourResult CustomMeshingBehaviour(int elementId, int typeId, ref double linearDeflection, ref double angularDeflection)
        {
            if (typeId == 571) // = reinforcingbar
            {
                linearDeflection *= 10;
                angularDeflection = 2.5;
            }
            return Xbim3DModelContext.MeshingBehaviourResult.Default;
        }

        private IModel ParseModelFile(string ifcFileName, bool caching, ILogger<BatchProcessor> logger, ReportProgressDelegate progress)
        {
            IModel ret = null;
            if (string.IsNullOrWhiteSpace(ifcFileName))
                return null;
            if (!File.Exists(ifcFileName))
                return null;
            // create a callback for progress
            var ext = Path.GetExtension(ifcFileName).ToLowerInvariant();
            switch (ext)
            {
                case ".ifc":
                case ".ifczip":
                case ".ifcxml":
                    if (caching)
                        ret = IfcStore.Open(ifcFileName, null, 0, progress);
                    else
                        ret = MemoryModel.OpenRead(ifcFileName, logger, progress);
                    return ret;
                default:
                    logger?.LogError("XbimRegression does not support {0} file format.", ext);
                    return null;
            }
        }


        private static string BuildFileName(string ifcFile, string extension)
        {
            return string.Concat(ifcFile, extension);
        }

        private void RemoveFiles(string ifcFile)
        {
            DeleteFile(BuildFileName(ifcFile, ".xbim"));
            DeleteFile(BuildFileName(ifcFile, ".xbimScene"));
            DeleteFile(BuildFileName(ifcFile, ".log"));
        }

        private void DeleteFile(string file)
        {
            try
            {
                if (File.Exists(file))
                    File.Delete(file);
            }
            catch (Exception)
            {
                // ignore
            }
        }


        private static long ReadFileLength(string file)
        {
            long length = 0;
            var fi = new FileInfo(file);
            if (fi.Exists)
            {
                length = fi.Length;
            }
            return length;
        }


        private static string SanitiseMessage(string message, string ifcFileName)
        {
            var modelPath = Path.GetDirectoryName(ifcFileName) ?? string.Empty;
            var currentPath = Environment.CurrentDirectory;

            return message
                .Replace(modelPath, string.Empty)
                .Replace(currentPath, string.Empty);
        }
    }
}