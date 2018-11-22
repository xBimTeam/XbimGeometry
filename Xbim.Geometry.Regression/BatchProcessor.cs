using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene;

namespace XbimRegression
{
    /// <summary>
    /// Class to process a folder of IFC files, capturing key metrics about their conversion
    /// </summary>
    public class BatchProcessor
    {
        private ILoggerFactory _loggerFactory;
        private ILogger _logger;

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
            _loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
           
            _logger = _loggerFactory.CreateLogger<BatchProcessor>();
            
            var di = new DirectoryInfo(Params.TestFileRoot);

            var resultsFile = Path.Combine(Params.TestFileRoot, string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));
            // We need to use the logger early to initialise before we use EventTrace
            _logger.LogDebug("Conversion starting...");
            using (var writer = new StreamWriter(resultsFile))
            {
                writer.WriteLine(ProcessResult.CsvHeader);
                // ParallelOptions opts = new ParallelOptions() { MaxDegreeOfParallelism = 12 };
                var toProcess = di.GetFiles("*.IFC", SearchOption.AllDirectories);
                // Parallel.ForEach<FileInfo>(toProcess, opts, file =>
                foreach (var file in toProcess)
                {
                    Console.WriteLine("Processing {0}", file);
                    var result = ProcessFile(file.FullName, writer);
                    if (!result.Failed)
                    {
                        Console.WriteLine("Processed {0} : {1} errors, {2} Warnings in {3}ms. {4} IFC Elements & {5} Geometry Nodes.",
                            file, result.Errors, result.Warnings, result.TotalTime, result.Entities, result.GeometryEntries);
                    }
                    else
                    {
                        Console.WriteLine("Processing failed for {0} after {1}ms.", file, result.TotalTime);
                    }
                }
                //);
                writer.Close();
            }
            Console.WriteLine("Finished. Press Enter to continue...");
            // LogManager.Shutdown();
            _logger = null;
            Console.ReadLine();
        }

        private ProcessResult ProcessFile(string ifcFile, StreamWriter writer)
        {
            RemoveFiles(ifcFile);
           // using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                var result = new ProcessResult() { Errors = -1 };
                try
                {
                    var watch = new Stopwatch();
                    watch.Start();
                    using (var model = ParseModelFile(ifcFile, Params.Caching))
                    {
                        var parseTime = watch.ElapsedMilliseconds;
                        var xbimFilename = BuildFileName(ifcFile, ".xbim");
                        var context = new Xbim3DModelContext(model);
                        if (_params.MaxThreads > 0)
                            context.MaxThreads = _params.MaxThreads;
                        // context.CustomMeshingBehaviour = CustomMeshingBehaviour;
                        context.CreateContext();
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
                                Application = ohs == null ? "Unknown" : ohs.OwningApplication.ToString(),
                            };
                        }
                        var xbim = Path.ChangeExtension(ifcFile, "xbim");
                        model.SaveAs(xbim);
                        model.Close();
                    }
                }

                catch (Exception ex)
                {
                    _logger.LogError(string.Format("Problem converting file: {0}", ifcFile), ex);
                    result.Failed = true;
                }
                finally
                {
                    //result.Errors = (from e in eventTrace.Events
                    //    where (e.EventLevel == EventLevel.ERROR)
                    //    select e).Count();
                    //result.Warnings = (from e in eventTrace.Events
                    //    where (e.EventLevel == EventLevel.WARN)
                    //    select e).Count();
                    result.FileName = ifcFile.Remove(0, Params.TestFileRoot.Length).TrimStart('\\');
                    //if (eventTrace.Events.Count > 0)
                    //{
                    //    CreateLogFile(ifcFile, eventTrace.Events);
                    //}
                    writer.WriteLine(result.ToCsv());
                    writer.Flush();
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
            return  Xbim3DModelContext.MeshingBehaviourResult.Default;
        }

        // todo: is caching going to come back?
        // ReSharper disable once UnusedParameter.Local
        private static IfcStore ParseModelFile(string ifcFileName, bool caching)
        {
            if (string.IsNullOrWhiteSpace(ifcFileName))
                return null;
            // create a callback for progress
            switch (Path.GetExtension(ifcFileName).ToLowerInvariant())
            {
                case ".ifc":
                case ".ifczip":
                case ".ifcxml":
                    return IfcStore.Open(ifcFileName, null, 0);
                default:
                    throw new NotImplementedException(
                        string.Format("XbimRegression does not support {0} file formats currently",
                            Path.GetExtension(ifcFileName))
                            );
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
