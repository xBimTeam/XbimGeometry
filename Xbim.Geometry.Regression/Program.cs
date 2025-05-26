using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using Xbim.Common;
using Xbim.Common.Configuration;
using Xbim.Ifc;

namespace XbimRegression
{
    class Program
    {
        private static void Main(string[] args)
        {
            // if running under vs I can create a batch to compare the result of the last run
            if (!string.IsNullOrEmpty(Environment.GetEnvironmentVariable("VisualStudioVersion")))
            {
                var loc = Assembly.GetEntryAssembly().Location;
                FileInfo f = new FileInfo(loc);
                var batchName = Path.ChangeExtension(f.FullName, ".bat");
                var batchContent = $"{f.Name} \"{string.Join("\" \"", args)}\"";

                File.WriteAllText(batchName, batchContent);
                Debug.WriteLine($"Executing in {f.FullName}");
            }
            // ContextTesting is a class that has been temporarily created to test multiple files
            // ContextTesting.Run();
            // return;

            var arguments = new Params(args);
            if (!arguments.IsValid)
                return;

            var processor = new BatchProcessor(arguments);
            processor.Run();
        }

    }
}
