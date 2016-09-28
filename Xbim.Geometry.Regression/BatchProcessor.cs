using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common.Logging;
using Xbim.Common.Step21;
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
        private ILogger Logger;
        
        Params _params;

        public BatchProcessor(Params arguments)
        {
            _params = arguments;

        }

        public Params Params
        {
            get
            {
                return _params;
            }
        }

        public void Run()
        {
            Logger = LoggerFactory.GetLogger();
            DirectoryInfo di = new DirectoryInfo(Params.TestFileRoot);

            String resultsFile = Path.Combine(Params.TestFileRoot, String.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));
            // We need to use the logger early to initialise before we use EventTrace
            Logger.Debug("Conversion starting...");
            using (StreamWriter writer = new StreamWriter(resultsFile))
            {
                writer.WriteLine(ProcessResult.CsvHeader);
               // ParallelOptions opts = new ParallelOptions() { MaxDegreeOfParallelism = 12 };
                FileInfo[] toProcess = di.GetFiles("*.IFC", SearchOption.AllDirectories);
               // Parallel.ForEach<FileInfo>(toProcess, opts, file =>
                foreach (var file in toProcess)
	
                {
                    Console.WriteLine("Processing {0}", file);
                    ProcessResult result = ProcessFile(file.FullName, writer);
                    if (!result.Failed)
                    {
                        Console.WriteLine("Processed {0} : {1} errors, {2} Warnings in {3}ms. {4} IFC Elements & {5} Geometry Nodes.",
                            file, result.Errors, result.Warnings, result.TotalTime, result.Entities, result.GeometryEntries);
                    }
                    else
                    {
                        Console.WriteLine("Processing failed for {0} after {1}ms.",
                            file, result.TotalTime);
                    }
                }
                //);
                writer.Close();
            }
            
            Console.WriteLine("Finished. Press Enter to continue...");
           // LogManager.Shutdown();
            Logger = null;
            Console.ReadLine();
        }

        private ProcessResult ProcessFile(string ifcFile, StreamWriter writer)
        {
            RemoveFiles(ifcFile);
            long geomTime = -1;  long parseTime = -1;
            using (EventTrace eventTrace = LoggerFactory.CreateEventTrace())
            {
                ProcessResult result = new ProcessResult() { Errors = -1 };
                try
                {

                    Stopwatch watch = new Stopwatch();
                    watch.Start();
                    using (var  model = ParseModelFile(ifcFile,Params.Caching))
                    {
                        parseTime = watch.ElapsedMilliseconds;
                        string xbimFilename = BuildFileName(ifcFile, ".xbim");
                        Xbim3DModelContext context = new Xbim3DModelContext(model);
                        context.CreateContext();
                        //}
                        geomTime = watch.ElapsedMilliseconds - parseTime;
                        //XbimSceneBuilder sb = new XbimSceneBuilder();
                        //string xbimSceneName = BuildFileName(ifcFile, ".xbimScene");
                        //sb.BuildGlobalScene(model, xbimSceneName);
                        // sceneTime = watch.ElapsedMilliseconds - geomTime;
                        IStepFileHeader header = model.Header;
                        watch.Stop();
                        IIfcOwnerHistory ohs = model.Instances.OfType<IIfcOwnerHistory>().FirstOrDefault();
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
                            IfcDescription = String.Format("{0}, {1}", header.FileDescription.Description.FirstOrDefault(), header.FileDescription.ImplementationLevel),
                            GeometryEntries = geomReader.ShapeInstances.Count(),
                            IfcLength = ReadFileLength(ifcFile),
                            XbimLength = ReadFileLength(xbimFilename),
                            SceneLength = 0,
                            IfcProductEntries = model.Instances.OfType<IIfcProduct>().Count(),
                            IfcSolidGeometries = model.Instances.OfType<IIfcSolidModel>().Count(),
                            IfcMappedGeometries = model.Instances.OfType<IIfcMappedItem>().Count(),
                            BooleanGeometries = model.Instances.OfType<IIfcBooleanResult>().Count(),
                            BReps = model.Instances.OfType<IIfcFaceBasedSurfaceModel>().Count() + model.Instances.OfType<IIfcShellBasedSurfaceModel>().Count() + model.Instances.OfType<IIfcManifoldSolidBrep>().Count(),
                            Application = ohs == null ? "Unknown" : ohs.OwningApplication.ToString(),
                            };
                        }
                        model.Close();                       
                    }
                }

                catch (Exception ex)
                {
                    Logger.Error(String.Format("Problem converting file: {0}", ifcFile), ex);
                    result.Failed = true;
                }
                finally
                {
                    result.Errors = (from e in eventTrace.Events
                                     where (e.EventLevel == EventLevel.ERROR)
                                     select e).Count();
                    result.Warnings = (from e in eventTrace.Events
                                       where (e.EventLevel == EventLevel.WARN)
                                       select e).Count();
                    result.FileName = ifcFile.Remove(0, Params.TestFileRoot.Length).TrimStart('\\');
                    if (eventTrace.Events.Count > 0)
                    {
                        CreateLogFile(ifcFile, eventTrace.Events);
                    }

                  
                        writer.WriteLine(result.ToCsv());
                        writer.Flush();
                   
                }
                return result;
            }
        }

        private static IfcStore ParseModelFile(string ifcFileName,bool caching)
        {
           
            //create a callback for progress
            switch (Path.GetExtension(ifcFileName).ToLowerInvariant())
            {
                case ".ifc":
                case ".ifczip":
                case ".ifcxml":
                    return IfcStore.Open(ifcFileName, null,0);
                  
                default:
                    throw new NotImplementedException(String.Format("XbimConvert does not support converting {0} file formats currently", Path.GetExtension(ifcFileName)));
            }
           
        }


        private static String BuildFileName(string ifcFile, string extension)
        {
            return String.Concat(ifcFile, extension);
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
            FileInfo fi = new FileInfo(file);
            if (fi.Exists)
            {
                length = fi.Length;
            }
            return length;
        }
        private void CreateLogFile(string ifcFile, IList<Event> events)
        {
            try
            {
                string logfile = String.Concat(ifcFile, ".log");
                using (StreamWriter writer = new StreamWriter(logfile, false))
                {
                    foreach (Event logEvent in events)
                    {
                        string message = SanitiseMessage(logEvent.Message, ifcFile);
                        writer.WriteLine("{0:yyyy-MM-dd HH:mm:ss} : {1:-5} {2}.{3} - {4}",
                            logEvent.EventTime,
                            logEvent.EventLevel.ToString(),
                            logEvent.Logger,
                            logEvent.Method,
                            message
                            );
                    }
                    writer.Flush();
                    writer.Close();
                }
            }
            catch (Exception e)
            {
                Logger.Error(String.Format("Failed to create Log File for {0}", ifcFile), e);
            }
        }

        private  string SanitiseMessage(string message, string ifcFileName)
        {
            string modelPath = Path.GetDirectoryName(ifcFileName);
            string currentPath = Environment.CurrentDirectory;

            return message
                .Replace(modelPath, String.Empty)
                .Replace(currentPath, String.Empty);
        }
    }

}
