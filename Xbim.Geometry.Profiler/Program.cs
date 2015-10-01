using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using Xbim.Common.Logging;
using Xbim.ModelGeometry.Scene;
using XbimGeometry.Interfaces;
using Xbim.IO.Esent;
using Xbim.Common;
using Xbim.Ifc2x3.IO;

namespace Xbim.Geometry.Profiler
{

    class Program
    {
        internal static readonly ILogger Logger = LoggerFactory.GetLogger();
        /// <summary>
        /// Converts an Ifc File to xBIM if it does not already exists, then converts the geoemtry to Xbim format and profiles the results
        /// </summary>
        /// <param name="args"> file[.ifc, xbim]</param>

        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("No Ifc or xBim file specified");
                return;
            }
            var fileName = args[0];
            var mainStopWatch = new Stopwatch();
            mainStopWatch.Start();
            using (var model = GetModel(fileName))
            {
                if (model != null)
                {
                    var functionStack = new ConcurrentStack<Tuple<string,double>>();
                    ReportProgressDelegate progDelegate = delegate(int percentProgress, object userState)
                    {
                        if (percentProgress == -1)
                        {
                            functionStack.Push(new Tuple<string,double>(userState.ToString(), DateTime.Now.TimeOfDay.TotalMilliseconds));
                            Logger.InfoFormat("Entering - {0}", userState.ToString());
                        }
                    
                        else if (percentProgress == 101)
                        {
                            Tuple<string,double> func; 
                            if(functionStack.TryPop(out func))
                                Logger.InfoFormat("Complete in \t\t{0:0.0} ms", DateTime.Now.TimeOfDay.TotalMilliseconds - func.Item2);
                        }
                    };
                    var context = new Xbim3DModelContext(model);
                    context.CreateContext(geomStorageType: XbimGeometryType.PolyhedronBinary, progDelegate: progDelegate);

                    mainStopWatch.Stop();
                    Logger.InfoFormat("Xbim total Compile Time \t\t{0:0.0} ms", mainStopWatch.ElapsedMilliseconds);
                    var wexBimFilename = Path.ChangeExtension(fileName, "wexBIM");
                    using (var wexBiMfile = new FileStream(wexBimFilename, FileMode.Create, FileAccess.Write))
                    {
                        using (var wexBimBinaryWriter = new BinaryWriter(wexBiMfile))
                        {
                            var stopWatch = new Stopwatch();
                            Logger.InfoFormat("Entering -  Create wexBIM");
                            stopWatch.Start();
                            context.Write(wexBimBinaryWriter);
                            stopWatch.Stop();
                            Logger.InfoFormat("Complete - in \t\t{0:0.0} ms", stopWatch.ElapsedMilliseconds);
                            wexBimBinaryWriter.Close();
                        }
                        wexBiMfile.Close();
                    }

                    model.Close();
                }
            }
            Console.WriteLine("Press any key to exit");
            Console.Read();
        }

        private static EsentModel GetModel(string fileName)
        {
            EsentModel openModel = null;
            var extension = Path.GetExtension(fileName);
            if (string.IsNullOrWhiteSpace(extension))
            {
                if (File.Exists(Path.ChangeExtension(fileName, "xbim"))) //use xBIM if exists
                    fileName = Path.ChangeExtension(fileName, "xbim");
                else if (File.Exists(Path.ChangeExtension(fileName, "ifc"))) //use ifc if exists
                    fileName = Path.ChangeExtension(fileName, "ifc");
                else if (File.Exists(Path.ChangeExtension(fileName, "ifczip"))) //use ifczip if exists
                    fileName = Path.ChangeExtension(fileName, "ifczip");
                else if (File.Exists(Path.ChangeExtension(fileName, "ifcxml"))) //use ifcxml if exists
                    fileName = Path.ChangeExtension(fileName, "ifcxml");
            }

            if (File.Exists(fileName))
            {
                extension = Path.GetExtension(fileName);
                if (String.Compare(extension, ".xbim", StringComparison.OrdinalIgnoreCase) == 0) //just open xbim
                {

                    try
                    {
                        var model = new XbimModel();
                        model.Open(fileName, XbimDBAccess.ReadWrite);
                        //delete any geometry
                        openModel = model;
                    }
                    catch (Exception e)
                    {
                        Logger.ErrorFormat("Unable to open model {0}, {1}", fileName, e.Message);
                        Console.WriteLine(String.Format("Unable to open model {0}, {1}", fileName, e.Message));
                    }

                }
                else //we need to create the xBIM file
                {
                    var model = new XbimModel();
                    try
                    {
                        model.CreateFrom(fileName, null, null, true);
                        openModel = model;
                    }
                    catch (Exception e)
                    {
                        Logger.ErrorFormat("Unable to open model {0}, {1}", fileName, e.Message);
                        Console.WriteLine(String.Format("Unable to open model {0}, {1}", fileName, e.Message));
                    }

                }
            }
            return openModel;
        }
    }
}
