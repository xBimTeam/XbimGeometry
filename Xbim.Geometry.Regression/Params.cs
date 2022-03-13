using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;



namespace XbimRegression
{
    /// <summary>
    /// Class representing the command line parameters sent to this application
    /// </summary>
    public class Params
    {
        public int MaxThreads;

        private const int DefaultTimeout = 1000 * 60 * 20; // 20 mins
        public bool Caching = false;
        public bool ReportProgress = false;
        public List<int> WriteBreps = null;
        public bool GeometryV1;

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
                FilesToProcess = di.GetFiles("*.IFC", SearchOption.AllDirectories).Where(y=>y.Extension.ToLowerInvariant() == ".ifc");
                ResultsFile = Path.Combine(TestFileRoot, string.Format("XbimRegression_{0:yyyyMMdd-hhmmss}.csv", DateTime.Now));
            }
            else if (File.Exists(TestFileRoot))
            {
                var ext = Path.GetExtension(TestFileRoot).ToLowerInvariant();
                if (ext == ".ifc")
                {
                    FilesToProcess = new[] { new FileInfo(TestFileRoot) };
                    ResultsFile = Path.ChangeExtension(TestFileRoot, "regression.csv");
                }
                else if (ext == ".txt")
                {
                    var justLines = File.ReadAllLines(TestFileRoot).Where(x => !x.StartsWith("#"));
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
                Console.WriteLine("Invalid model folder {0}", TestFileRoot);
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
                            case "/writebreps":
                            case "/breps":
                            case "/brep":
                                WriteBreps = WriteBreps ?? new List<int>();
                                paramType = CompoundParameter.Breps;
                                break;
                            case "/timeout":
                                paramType = CompoundParameter.Timeout;
                                break;
                            case "/maxthreads":
                                paramType = CompoundParameter.MaxThreads;
                                break;
                            case "/caching":
                                Caching = true;
                                break;
                            case "/progress":
                                ReportProgress = true;
                                break;
                            case "/geometryv1":
                                GeometryV1 = true;
                                break;
                            default:
                                Console.WriteLine("Skipping un-expected argument '{0}'", arg);
                                break;
                        }
                        break;
                    case CompoundParameter.Timeout:
                        int timeout;
                        if (int.TryParse(arg, out timeout))
                        {
                            Timeout = timeout * 1000;
                        }
                        paramType = CompoundParameter.None;
                        break;
                    case CompoundParameter.MaxThreads:
                        int mt;
                        if (int.TryParse(arg, out mt))
                        {
                            MaxThreads = mt;
                        }
                        paramType = CompoundParameter.None;
                        break;
                    case CompoundParameter.Breps:
                        int brepv;
                        if (int.TryParse(arg, out brepv))
                        {
                            WriteBreps.Add(brepv);
                        }
                        else
						{
                            paramType = CompoundParameter.None;
                            i--;
                        }
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
        public IEnumerable<FileInfo> FilesToProcess { get; private set; } = Enumerable.Empty<FileInfo>();
        public string ResultsFile { get; }

        private enum CompoundParameter
        {
            None,
            Timeout,
            MaxThreads,
            CachingOn,
            Breps
        };
    }
}
