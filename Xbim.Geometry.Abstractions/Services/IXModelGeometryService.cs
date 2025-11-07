using System;
using System.Collections.Generic;
using Xbim.Common;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXModelGeometryService
    {

        IXLoggingService LoggingService { get; }
        IXVertexFactory VertexFactory { get; }
        IXGeometryFactory GeometryFactory { get; }
        IXCurveFactory CurveFactory { get; }
        IXSurfaceFactory SurfaceFactory { get; }
        IXEdgeFactory EdgeFactory { get; }
        IXWireFactory WireFactory { get; }
        IXFaceFactory FaceFactory { get; }
        IXShellFactory ShellFactory { get; }
        IXSolidFactory SolidFactory { get; }
        IXCompoundFactory CompoundFactory { get; }
        IXBooleanFactory BooleanFactory { get; }
        IXShapeFactory ShapeFactory { get; }
        IXProfileFactory ProfileFactory { get; }
        IXMaterialFactory MaterialFactory { get; }
        IXProjectionFactory ProjectionFactory { get; }
        IXWexBimMeshFactory WexBimMeshFactory { get; }
        IXShapeBinarySerializer ShapeBinarySerializer { get; }
        IXModelPlacementBuilder ModelPlacementBuilder { get; }

        double Timeout { get; set; }
        /// <summary>
        /// The distance between two points less than or equal to which they are determined to be coincidental
        /// </summary>
        double Precision { get; }
        /// <summary>
        /// Precision * Precision
        /// </summary>
        double PrecisionSquared { get; }
        /// <summary>
        /// Model units that make up one meter
        /// </summary>
        double OneMeter { get; }
        /// <summary>
        /// Model units that make up one foot
        /// </summary>
        double OneFoot { get; }
        /// <summary>
        /// Model units that make up one millimeter
        /// </summary>
        double OneMillimeter { get; }
        /// <summary>
        /// The smallest distance between faces and edges before they are determined to be coincidental, 
        /// it is in millimeters the default = 1.0mm
        /// Mostly only relevant to Boolean operations
        /// </summary>
        double MinimumGap { get; set; }
        /// <summary>
        /// The factor to convert model angles to radians, for a radian unit this = 1 and for a degree unit = PI/180;
        /// </summary>
        double RadianFactor { get; }

        /// <summary>
        /// The smallest area of a closed wire in square metres, wires with areas below this are regarded as lines
        /// </summary>
        double MinAreaM2 { get; }
        /// <summary>
        /// Returns a copy of the default mesh factors for the model
        /// </summary>
        IXMeshFactors MeshFactors { get; }
        // IfcType InstancesOf<IfcType>();
        IModel Model { get; }
        /// <summary>
        /// Initialise with a model
        /// </summary>
        /// <param name="model"></param>       
        void SetModel(IModel model);

        /// <summary>
        /// Gets a set of 3d Gemtric representation contexts, is adaptive to queiks in how models have been set up by different vendors
        /// </summary>
        /// <returns></returns>
        ISet<IIfcGeometricRepresentationContext> GetTypical3dContexts();
        /// <summary>
        /// Set to true to try and upgrade FaceSet models to correct shells or solids that may have definition errors
        /// </summary>
        bool UpgradeFaceSets { get; set; }

        IXLocation Create(IIfcObjectPlacement objectPlacement);
        /// <summary>
        /// Creates a location representing the transformation in the mapped item
        /// </summary>
        /// <param name="mappedItem"></param>
        /// <returns></returns>
        IXLocation CreateMappingTransform(IIfcMappedItem mappedItem);


        void LogError(string format, params object[] args);
        void LogError(IPersistEntity ifcEntity, string format, params object[] args);
        void LogError(IPersistEntity ifcEntity, Exception exception, string format, params object[] args);
        void LogError(Exception exception, string format, params object[] args);

        void LogWarning(string format, params object[] args);
        void LogWarning(IPersistEntity ifcEntity, string format, params object[] args);
        void LogWarning(IPersistEntity ifcEntity, Exception exception, string format, params object[] args);
        void LogWarning(Exception exception, string format, params object[] args);

        void LogInformation(string format, params object[] args);
        void LogInformation(IPersistEntity ifcEntity, string format, params object[] args);
        void LogInformation(IPersistEntity ifcEntity, Exception exception, string format, params object[] args);
        void LogInformation(Exception exception, string format, params object[] args);

        void LogDebug(string format, params object[] args);
        void LogDebug(IPersistEntity ifcEntity, string format, params object[] args);
        void LogDebug(IPersistEntity ifcEntity, Exception exception, string format, params object[] args);
        void LogDebug(Exception exception, string format, params object[] args);
    }
}
