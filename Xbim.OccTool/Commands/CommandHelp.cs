using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Xbim.OccTool.Commands
{
    class CommandHelp : Command
    {
        internal static string Name = "help";

        internal static string Syntax = "help [command]";

        internal override string GetName()
        {
            return Name;
        }

        internal override string GetSyntax()
        {
            return Syntax;
        }

        internal override int Process(string[] args, int index)
        {
            var consumed = 0;
            var helptopic = "help";
            if (index < args.Length)
            {
                helptopic = args[index];
                consumed++;
            }

            switch (helptopic)
            {
                case "brep":
                    Console.WriteLine(string.Format("Use '{0}' for instructions on a specific command.", CommandHelp.Syntax));
                    break;
                default:
                    Console.WriteLine(string.Format("Syntax is: {0}", CommandBrep.Syntax));
                    break;
            }
            return consumed;           
        }
    }
}
