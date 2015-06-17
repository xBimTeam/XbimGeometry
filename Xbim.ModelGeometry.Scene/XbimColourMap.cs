using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using Xbim.Ifc2x3.ActorResource;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.PresentationAppearanceResource;
using Xbim.Ifc2x3.ProductExtension;
using Xbim.Ifc2x3.SharedBldgElements;
using Xbim.Ifc2x3.SharedBldgServiceElements;

namespace Xbim.ModelGeometry.Scene
{
    public enum StandardColourMaps
    {
        /// <summary>
        /// Creates a colour map based on the IFC product types
        /// </summary>
        IfcProductTypeMap,
        Federation,
        /// <summary>
        /// Creates an empty colour map
        /// </summary>
        Empty
    }

    /// <summary>
    /// Provides a map for obtaining a colour for a keyed type, the colour is an ARGB value
    /// </summary>
    [DataContract(Name="ColourMap")]
    public class XbimColourMap : KeyedCollection<string, XbimColour>
    {

        public override int GetHashCode()
        {
            int hash = 0;
            foreach (var colour in this)
            {
                hash ^= colour.GetHashCode();
            }
            return hash;
        }

        public override bool Equals(object obj)
        {
            XbimColourMap map = obj as XbimColourMap;
            if (map == null) return false;
            if(map.Count!=Count) return false;
            foreach (var colour in map)
            {
                if (!this.Contains(colour)) return false;
                if (!colour.Equals( this[colour.Name])) return false;
            }
            return true;
        }


        protected override string GetKeyForItem(XbimColour item)
        {
            return item.Name;
        }

        [IgnoreDataMember]
        public bool IsTransparent
        {
            get
            {
                return this.Any(c => c.IsTransparent);
            }
        }

        public XbimColourMap(StandardColourMaps initMap= StandardColourMaps.IfcProductTypeMap)
        {
            switch (initMap)
            {
                case StandardColourMaps.IfcProductTypeMap:
                    SetProductTypeColourMap();
                    break;
                case StandardColourMaps.Federation:
                    SetFederationRoleColourMap();
                    break;
                default:
                    break;
            }
        }

        public void SetFederationRoleColourMap()
        {
            Clear();
            Add(new XbimColour("Default", 0.98, 0.92, 0.74, 1)); //grey

            // previously assigned colors
            Add(new XbimColour(IfcRole.Architect.ToString(), 1.0 , 1.0 , 1.0 , .5)); //white
            Add(new XbimColour(IfcRole.MechanicalEngineer.ToString(), 1.0, 0.5, 0.25, 1));
            Add(new XbimColour(IfcRole.ElectricalEngineer.ToString(), 0.0, 0, 1.0, 1)); //blue
            Add(new XbimColour(IfcRole.StructuralEngineer.ToString(), 0.2, 0.2, 0.2, 1.0)); //dark

            // new colours assigned from wheel
            double WheelAngle = 0;
            Add(XbimColour.FromHSV(IfcRole.BuildingOperator.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.BuildingOwner.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.CivilEngineer.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Client.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.ComissioningEngineer.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.ConstructionManager.ToString(), WheelAngle += 15, 1, 1)); 
            Add(XbimColour.FromHSV(IfcRole.Consultant.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Contractor.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.CostEngineer.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Engineer.ToString(), WheelAngle += 15, 1, 1)); 
            Add(XbimColour.FromHSV(IfcRole.FacilitiesManager.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.FieldConstructionManager.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Manufacturer.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Owner.ToString(), WheelAngle += 15, 1, 1)); 
            Add(XbimColour.FromHSV(IfcRole.ProjectManager.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Reseller.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Subcontractor.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.Supplier.ToString(), WheelAngle += 15, 1, 1));
            Add(XbimColour.FromHSV(IfcRole.UserDefined.ToString(), WheelAngle += 15, 1, 1));
        }
       
        new public XbimColour this[string key]
        {
            get
            {
                if (base.Contains(key))
                    return base[key];
                if (base.Contains("Default"))
                    return base["Default"];
                return XbimColour.Default;
            }
        }

        public void SetProductTypeColourMap()
        {
            Clear();
            Add(new XbimColour("Default", 0.98, 0.92, 0.74, 1));
            Add(new XbimColour(typeof(IfcWall).Name, 0.98, 0.92, 0.74, 1));
            Add(new XbimColour(typeof(IfcWallStandardCase).Name, 0.98, 0.92, 0.74, 1));
            Add(new XbimColour(typeof(IfcRoof).Name, 0.28, 0.24, 0.55, 1));
            Add(new XbimColour(typeof(IfcBeam).Name, 0.0, 0.0, 0.55, 1));
            Add(new XbimColour(typeof(IfcBuildingElementProxy).Name, 0.95, 0.94, 0.74, 1));
            Add(new XbimColour(typeof(IfcColumn).Name, 0.0, 0.0, 0.55, 1));
            Add(new XbimColour(typeof(IfcSlab).Name, 0.47, 0.53, 0.60, 1));
            Add(new XbimColour(typeof(IfcWindow).Name, 0.68, 0.85, 0.90, 0.5));
            Add(new XbimColour(typeof(IfcCurtainWall).Name, 0.68, 0.85, 0.90, 0.4));
            Add(new XbimColour(typeof(IfcPlate).Name, 0.68, 0.85, 0.90, 0.4));
            Add(new XbimColour(typeof(IfcDoor).Name, 0.97, 0.19, 0, 1));
            Add(new XbimColour(typeof(IfcSpace).Name, 0.68, 0.85, 0.90, 0.4));
            Add(new XbimColour(typeof(IfcMember).Name, 0.34, 0.34, 0.34, 1));
            Add(new XbimColour(typeof(IfcDistributionElement).Name, 0.0, 0.0, 0.55, 1));
            Add(new XbimColour(typeof(IfcElectricalElement).Name, 0.0, 0.9, 0.1, 1));
            Add(new XbimColour(typeof(IfcFurnishingElement).Name, 1, 0, 0, 1));
            Add(new XbimColour(typeof(IfcOpeningElement).Name, 0.200000003, 0.200000003, 0.800000012, 0.2));
            Add(new XbimColour(typeof(IfcFeatureElementSubtraction).Name, 1.0, 1.0, 1.0, 0.0));
            Add(new XbimColour(typeof(IfcFlowTerminal).Name, 0.95, 0.94, 0.74, 1));
            Add(new XbimColour(typeof(IfcFlowSegment).Name, 0.95, 0.94, 0.74, 1));
            Add(new XbimColour(typeof(IfcDistributionFlowElement).Name, 0.95, 0.94, 0.74, 1));
            Add(new XbimColour(typeof(IfcFlowFitting).Name, 0.95, 0.94, 0.74, 1));
            Add(new XbimColour(typeof(IfcRailing).Name, 0.95, 0.94, 0.74, 1));
        }
    }
}
