using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
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
            _logger = LoggerFactory.GetLogger();
            var di = new DirectoryInfo(Params.TestFileRoot);

            var resultsFile = Path.Combine(Params.TestFileRoot, string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));
            // We need to use the logger early to initialise before we use EventTrace
            _logger.Debug("Conversion starting...");
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
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                var result = new ProcessResult() { Errors = -1 };
                try
                {
                    var watch = new Stopwatch();
                    watch.Start();
					Xbim.Common.ReportProgressDelegate del = null;
					if (_params.Progress)
						del = report;
					using (var model = ParseModelFile(ifcFile, Params.Caching, del))
                    {
                        var parseTime = watch.ElapsedMilliseconds;
                        var xbimFilename = BuildFileName(ifcFile, ".xbim");
                        var context = new Xbim3DModelContext(model);
                        if (_params.MaxThreads > 0)
                            context.MaxThreads = _params.MaxThreads;
                        
						if (_params.Progress)
							context.CreateContext(report);
						else
							context.CreateContext();
						
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

						if (_params.WriteBreps)
						{
							var path =
								   Path.Combine(
									   Path.GetDirectoryName(ifcFile),
									   Path.GetFileName(ifcFile) + ".brep.xbim4"
									   );
							IXbimGeometryEngine engine = new XbimGeometryEngine();
							if (!Directory.Exists(path))
								Directory.CreateDirectory(path);
							IfcStore s = model as IfcStore;
							if (s != null)
							{
								var types = new[]
								{
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
								foreach (var type in types)
								{
									foreach (var ent in s.Instances.OfType(type, false))
									{
										try
										{
											Xbim.Common.Geometry.IXbimGeometryObject created = null;
											if (ent is IIfcGeometricRepresentationItem)
												created = engine.Create((IIfcGeometricRepresentationItem)ent);
											if (ent is IIfcConnectedFaceSet)
												created = engine.CreateShell((IIfcConnectedFaceSet)ent);
											// IIfcConnectedFaceSet
											if (created != null)
											{
												var brep = engine.ToBrep(created);
												var brepFileName = Path.Combine(path, $"{ent.EntityLabel}.{type}.brep");
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
						}
						if (_params.Caching)
						{
							if (_params.Progress)
								Console.Write("Starting saving cache...");
							var xbim = Path.ChangeExtension(ifcFile, "xbim");
							model.SaveAs(xbim);
							if (_params.Progress)
								Console.WriteLine(" Complete.");
						}
						if (_params.Progress)
							Console.Write("Closing...");
						model.Close();
						if (_params.Progress)
							Console.WriteLine(" Complete.");
					}
                }

                catch (Exception ex)
                {
                    _logger.Error(string.Format("Problem converting file: {0}", ifcFile), ex);
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

		private void report(int percentProgress, object userState)
		{
			if (percentProgress < 0 || percentProgress > 100)
				return;
			Console.WriteLine($"{userState}: {percentProgress}%");
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
        private static IfcStore ParseModelFile(string ifcFileName, bool caching, Xbim.Common.ReportProgressDelegate prog)
        {
            if (string.IsNullOrWhiteSpace(ifcFileName))
                return null;
            // create a callback for progress
            switch (Path.GetExtension(ifcFileName).ToLowerInvariant())
            {
                case ".ifc":
                case ".ifczip":
                case ".ifcxml":
                    return IfcStore.Open(ifcFileName, null, 0, prog);
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

        private void CreateLogFile(string ifcFile, IList<Event> events)
        {
            try
            {
                var logfile = string.Concat(ifcFile, ".log");
                using (var writer = new StreamWriter(logfile, false))
                {
                    foreach (var logEvent in events)
                    {
                        var message = SanitiseMessage(logEvent.Message, ifcFile);
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
                _logger.Error(string.Format("Failed to create Log File for {0}", ifcFile), e);
            }
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
