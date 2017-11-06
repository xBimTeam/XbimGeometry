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
                    Console.WriteLine($"Use '{CommandHelp.Syntax}' for instructions on a specific command.");
                    break;
                default:
                    Console.WriteLine($"Syntax is: {CommandBrep.Syntax}");
                    break;
            }
            return consumed;           
        }
    }
}
