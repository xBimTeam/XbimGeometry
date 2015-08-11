using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace XbimRegression
{
    /// <summary>
    /// Class summarising the results of a model conversion
    /// </summary>
    public class ProcessResult
    {
        public ProcessResult()
        {
            Errors = -1;
        }
        public bool Failed { get; set; }
        public String FileName { get; set; }
        public int Errors { get; set; }
        public int Warnings { get; set; }
        public long ParseDuration { get; set; }
        public long GeometryDuration { get; set; }
        public long SceneDuration { get; set; }
        public long XbimLength { get; set; }
        public long SceneLength { get; set; }
        public long IfcLength { get; set; }
        public long Entities { get; set; }
        public long GeometryEntries { get; set; }
        public String IfcSchema { get; set; }
        public String IfcName { get; set; }
        public String IfcDescription { get; set; }
        public long IfcProductEntries { get; set; }
        public long IfcSolidGeometries { get; set; }
        public long IfcMappedGeometries { get; set; }
        public long BReps { get; set; }
        public String Application { get; set; }
        public long BooleanGeometries { get; set; }
        public const String CsvHeader = @"IFC File, Errors, Warnings, Parse Duration (ms), Geometry Conversion (ms), Total Duration (ms), IFC Size, Xbim Size, IFC Entities, Geometry Nodes, " +
           
            "FILE_SCHEMA, FILE_NAME, FILE_DESCRIPTION, Products, Solid Models, Maps, Booleans, Application";

        public String ToCsv()
        {
            return String.Format("\"{0}\",{1},{2},{3},{4},{6},{7},{8},{10},{11},\"{12}\",\"{13}\",\"{14}\",{15},{16},{17},{18},{19},\"{20}\"",
                FileName,           // 0
                Errors,             // 1
                Warnings,           // 2
                ParseDuration,      // 3
                GeometryDuration,   // 4
                SceneDuration,      // 5
                TotalTime,          // 6
                IfcLength,          // 7
                XbimLength,         // 8
                SceneLength,        // 9
                Entities,           // 10
                GeometryEntries,    // 11
                IfcSchema,          // 12
                IfcName,            // 13
                IfcDescription,     // 14
                IfcProductEntries,  // 15
                IfcSolidGeometries, // 16
                IfcMappedGeometries,// 17
                BooleanGeometries,  // 18
                BReps,              // 19
                Application         // 20
                );
        }

        public long TotalTime 
        {
            get
            {
                return ParseDuration + GeometryDuration + SceneDuration;
            }
        
        }



       
    }
}
