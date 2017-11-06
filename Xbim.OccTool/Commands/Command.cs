using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Xbim.OccTool.Commands
{
    abstract class Command
    {
        internal static string Syntax;
        internal static string Name;
        abstract internal string GetSyntax();


        abstract internal string GetName();
        
        
        abstract internal int Process(string[] args, int index);           
    }
}
