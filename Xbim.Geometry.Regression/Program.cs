using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using Xbim.Common;
using Xbim.Common.Configuration;
using Xbim.Ifc;

namespace XbimRegression
{
    class Program
    {
        private static void Main(string[] args)
        {

            // ContextTesting is a class that has been temporarily created to test multiple files
            // ContextTesting.Run();
            // return;
            
            //XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(opt => opt.AddHeuristicModel()));
            var arguments = new Params(args);
            if (!arguments.IsValid)
                return;
            
            var processor = new BatchProcessor(arguments);
            processor.Run();
        }

    }
}
