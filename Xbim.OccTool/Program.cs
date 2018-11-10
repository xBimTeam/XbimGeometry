using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Logging;
using Xbim.OccTool.Commands;

namespace Xbim.OccTool
{
    class Program
    {
        internal static readonly ILogger Log = LoggerFactory.GetLogger();

        static void Main(string[] args)
        {
            List<Command> commands = new List<Command>()
            {
                new CommandHelp(),
                new CommandBrep()
            };

            Console.WriteLine("Xbim.OccTool");
            if (args.Length == 0)
            {
                Console.WriteLine("- warning: arguments missing.");
                DisplayHelp(commands);
                return;
            }
            int index = 0;
            while (index < args.Length)
            {
                var processed = false;
                foreach (var command in commands)
                {
                    var cmName = command.GetName().ToLowerInvariant();
                    if (cmName == args[index].ToLowerInvariant())
                    {
                        index++;
                        // start evaluation of parameters at the next argument
                        index += command.Process(args, index);
                        processed = true;
                        break;
                    }
                }
                if (!processed)
                {
                    Log.Warn(string.Format("Parameter '{0}' ignored.", args[index]));
                    index++;
                }
            }
#if DEBUG
            Console.WriteLine("Press any key");
#else
            Console.WriteLine("Completed.");
#endif
            Console.ReadKey();
        }

        private static void DisplayHelp(List<Command> commands)
        {
            Console.WriteLine("Syntax is: Xbim.OccTool Command");
            Console.WriteLine("Available commands are:");
            foreach (var command in commands)
            {
                Console.WriteLine(string.Format("- {0}", command.GetSyntax()));
            }
            Console.WriteLine("use 'help CommandName' for more.");
        }
    }
}
