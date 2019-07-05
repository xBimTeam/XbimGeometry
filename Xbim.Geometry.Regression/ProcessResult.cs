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
        public int Information { get; set; }
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
        public const String CsvHeader = @"IFC File, Errors, Warnings, Information, Parse Duration (ms), Geometry Conversion (ms), Total Duration (ms), IFC Size,  IFC Entities, Geometry Nodes, " +
           
            "FILE_SCHEMA, FILE_NAME, FILE_DESCRIPTION, Application, Products, Solid Models, Maps, Booleans, BReps";

        public String ToCsv()
        {
            return String.Format($"\"{FileName}\",{Errors},{Warnings},{Information},{ParseDuration},{GeometryDuration},{TotalTime},{IfcLength},{Entities},{GeometryEntries},\"{IfcSchema}\",\"{IfcName}\",\"{IfcDescription}\",\"{Application}\",{IfcProductEntries},{IfcSolidGeometries},{IfcMappedGeometries},{BooleanGeometries},{BReps}");
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
