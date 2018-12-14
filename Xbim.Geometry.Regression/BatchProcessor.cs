using Microsoft.Extensions.Logging;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common;
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
            var resultsFile = Path.Combine(Params.TestFileRoot, string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));

            var di = new DirectoryInfo(Params.TestFileRoot);

            using (var writer = new StreamWriter(resultsFile))
            {
                writer.WriteLine(ProcessResult.CsvHeader);
                // ParallelOptions opts = new ParallelOptions() { MaxDegreeOfParallelism = 12 };
                var toProcess = di.GetFiles("*.IFC", SearchOption.AllDirectories);
                // Parallel.ForEach<FileInfo>(toProcess, opts, file =>
                foreach (var file in toProcess)
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
                    XbimLogging.LoggerFactory = null; // uses a default loggerFactory

                    var txt = File.ReadAllText(logFile);
                    if (string.IsNullOrEmpty(txt))
                    {
                        File.Delete(logFile);
                        result.Errors = 0;
                        result.Warnings = 0;
                    }
                    else
                    {

                        var tokens = txt.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                        result.Errors = tokens.Count(t => t == "\"Error\"");
                        result.Warnings = tokens.Count(t => t == "\"Warning\"");
                    }
                    if (result!=null && !result.Failed)
                    {
                        Console.WriteLine($"Processed {file} : {result.Errors} errors, {result.Warnings} Warnings in {result.TotalTime}ms. {result.Entities} IFC Elements & {result.GeometryEntries} Geometry Nodes." );
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
            }
            Console.WriteLine("Finished. Press Enter to continue...");
           
            Console.ReadLine();
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
                    
                    watch.Start();
                    using (var model = ParseModelFile(ifcFile, Params.Caching, logger))
                    {
                        var parseTime = watch.ElapsedMilliseconds;
                        var xbimFilename = BuildFileName(ifcFile, ".xbim");
                        var context = new Xbim3DModelContext(model, logger: logger);
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
                                Application = ohs == null ? "Unknown" : ohs.OwningApplication?.ApplicationFullName.ToString()
                            };

                        }
                    }
                }

                catch (Exception ex)
                {
                    logger.LogError(string.Format("Problem converting file: {0}", ifcFile), ex);
                    result.Failed = true;
                    result.GeometryDuration = watch.ElapsedMilliseconds / 1000;
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

        // todo: is caching going to come back?
        // ReSharper disable once UnusedParameter.Local
        private IModel ParseModelFile(string ifcFileName, bool caching, ILogger<BatchProcessor> logger)
        {
            if (string.IsNullOrWhiteSpace(ifcFileName))
                return null;
            // create a callback for progress
            switch (Path.GetExtension(ifcFileName).ToLowerInvariant())
            {
                case ".ifc":
                case ".ifczip":
                case ".ifcxml":
                    return MemoryModel.OpenRead(ifcFileName, logger);
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
