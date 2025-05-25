using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Xbim.Geometry.Abstractions;



namespace XbimRegression
{
    /// <summary>
    /// Class representing the command line parameters sent to this application
    /// </summary>
    public class Params
    {
        public int MaxThreads { get; set; }

        private const int DefaultTimeout = 1000 * 60 * 20; // 20 mins
        public bool Caching { get; set; } = false;
        public string CachingExtension { get; set; } = string.Empty;
        public bool AdjustWcs = true;
        public bool ReportProgress = false;
        public List<int> WriteBreps = null;

        public XGeometryEngineVersion EngineVersion { get; set; } = XGeometryEngineVersion.V6;

        public Params(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("Invalid number of Parameters, model folder required");
                WriteSyntax();
                return;
            }

            TestFileRoot = args[0];

            // work out what are the files to process and the report file
            //
            if (Directory.Exists(TestFileRoot))
            {
                var di = new DirectoryInfo(TestFileRoot);
                FilesToProcess = di.GetFiles("*.IFC", SearchOption.AllDirectories).Where(y => y.Extension.ToLowerInvariant() == ".ifc");
                ResultsFile = Path.Combine(TestFileRoot, string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));
            }
            else if (File.Exists(TestFileRoot))
            {
                var ext = Path.GetExtension(TestFileRoot).ToLowerInvariant();
                if (ext == ".ifc")
                {
                    FilesToProcess = new List<FileInfo>() { new FileInfo(TestFileRoot) };
                    ResultsFile = Path.ChangeExtension(TestFileRoot, "regression.csv");
                }
                else if (ext == ".txt")
                {
                    var justLines = File.ReadAllLines(TestFileRoot).Where(x => !x.StartsWith("#"));
                    foreach (var oneLine in justLines)
                    {
                        if (!File.Exists(oneLine))
                            Console.WriteLine($"File '{oneLine}' not found.");
                    }
                    FilesToProcess = justLines.Where(name => File.Exists(name)).Select(x => new FileInfo(x)).ToArray();
                    ResultsFile = string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now);
                }
                else
                {
                    Console.WriteLine("Invalid source file {0}", TestFileRoot);
                    return;
                }
            }
            else
            {
                Console.WriteLine("Invalid source folder {0}", TestFileRoot);
                Console.WriteLine("- Is directory: {0}", Directory.Exists(TestFileRoot));
                Console.WriteLine("- Is file: {0}", File.Exists(TestFileRoot));
                return;
            }

            Timeout = DefaultTimeout;
            CompoundParameter paramType = CompoundParameter.None;

            var eval = args.Skip(1).ToList();
            for (int i = 0; i < eval.Count; i++)
            {
                string arg = eval[i];
                switch (paramType)
                {
                    case CompoundParameter.None:
                        switch (arg.ToLowerInvariant())
                        {
                            case "/singlethread":
                                MaxThreads = 1;
                                break;
                            case "/lowthreadscount":
                                MaxThreads = Environment.ProcessorCount / 2;
                                break;
                            case "/nowcsadjust":
                                AdjustWcs = false;
                                break;
                            case "/writebreps":
                            case "/breps":
                            case "/brep":
                                WriteBreps ??= new List<int>();
                                paramType = CompoundParameter.Breps;
                                break;
                            case "/timeout":
                                paramType = CompoundParameter.Timeout;
                                break;
                            case "/maxthreads":
                                paramType = CompoundParameter.MaxThreads;
                                break;
                            case "/caching":
                                paramType = CompoundParameter.CachingExtension;
                                Caching = true;
                                break;
                            case "/engine":
                                paramType = CompoundParameter.GeometryEngine;
                                break;
                            case "/progress":
                                ReportProgress = true;
                                break;
                            case "/geometryv1":
                                Console.WriteLine("Obsolete argument /geometryv1 ignored");
                                break;
                            default:
                                Console.WriteLine("Skipping un-expected argument '{0}'", arg);
                                break;
                        }
                        break;
                    case CompoundParameter.Timeout:
                        if (int.TryParse(arg, out int timeout))
                        {
                            Timeout = timeout * 1000;
                        }
                        paramType = CompoundParameter.None;
                        break;
                    case CompoundParameter.MaxThreads:
                        if (int.TryParse(arg, out int mt))
                        {
                            MaxThreads = mt;
                        }
                        paramType = CompoundParameter.None;
                        break;
                    case CompoundParameter.CachingExtension:
                        if (arg.Contains('/')) // starts with or contains a /
                            i--; // re-evaluate as parameter
                        else
                            CachingExtension = arg;
                        paramType = CompoundParameter.None;
                        break;
                    case CompoundParameter.Breps:
                        if (int.TryParse(arg, out int brepv))
                        {
                            WriteBreps.Add(brepv);
                        }
                        else
                        {
                            paramType = CompoundParameter.None;
                            i--;
                        }
                        break;
                    case CompoundParameter.GeometryEngine:
                        if (arg.Equals("v5", StringComparison.OrdinalIgnoreCase))
                            EngineVersion = XGeometryEngineVersion.V5;
                        else if (arg.Equals("v6", StringComparison.OrdinalIgnoreCase))
                            EngineVersion = XGeometryEngineVersion.V6;
                        else
                            Console.WriteLine($"Invalid geometry engine version '{arg}', expected 'v5' or 'v6'. Defaulting to v6.");
                        paramType = CompoundParameter.None;
                        break;
                }
            }
            IsValid = true;
        }

        private static void WriteSyntax()
        {
            Console.WriteLine("Syntax: XbimRegression <modelfolder> [/timeout <seconds>] [/maxthreads <number>] [/singlethread] /writebreps [labels]");
        }

        /// <summary>
        /// The folder root to locate the IFC test files in
        /// </summary>
        public String TestFileRoot { get; set; }

        /// <summary>
        /// Timeout duration, in milli-seconds
        /// </summary>
        public int Timeout { get; set; }

        /// <summary>
        /// Flag indicating if the parameters are valid
        /// </summary>
        public bool IsValid { get; set; }
        public IEnumerable<FileInfo> FilesToProcess { get; private set; } = new List<FileInfo>();
        public string ResultsFile { get; }

        private enum CompoundParameter
        {
            None,
            Timeout,
            MaxThreads,
            CachingExtension,
            GeometryEngine,
            Breps
        };
    }
}
