using System;
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
        public bool Caching;
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

            if (!Directory.Exists(TestFileRoot))
            {
                Console.WriteLine("Invalid model folder {0}", TestFileRoot);
                return;
            }

            Timeout = DefaultTimeout;

            CompoundParameter paramType = CompoundParameter.None;

            foreach (string arg in args.Skip(1))
            {
                switch (paramType)
                {
                    case CompoundParameter.None:
                        switch (arg.ToLowerInvariant())
                        {
                            case "/singlethread":
                                MaxThreads = 1;
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
                }
                
            }

            IsValid = true;

        }

        private static void WriteSyntax()
        {
            Console.WriteLine("Syntax: XbimRegression <modelfolder> [/timeout <seconds>]");
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


        private enum CompoundParameter
        {
            None,
            Timeout,
            MaxThreads,
            CachingOn
        };
    }

     
}
