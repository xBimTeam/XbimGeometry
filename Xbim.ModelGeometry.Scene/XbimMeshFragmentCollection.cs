using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimMeshFragmentCollection : List<XbimMeshFragment>
    {
       
        public XbimMeshFragmentCollection(XbimMeshFragmentCollection value)
            :base(value)
        {
           
        }
        public XbimMeshFragmentCollection()
        {

        }
        /// <summary>
        /// Returns true if the collection contains a fragment for the specified type
        /// </summary>
        /// <returns></returns>
        public bool Contains<T>()
        {
            foreach (var frag in this)
                if(typeof(T).IsAssignableFrom (frag.EntityType ))
                    return true;
            return false;
        }

        public XbimMeshFragmentCollection OfType<T>()
        {
            XbimMeshFragmentCollection results = new XbimMeshFragmentCollection();
            results.AddRange( this.Where(frag=>typeof(T).IsAssignableFrom(frag.EntityType)));
            return results;
        }

        public XbimMeshFragmentCollection Excluding<T>()
        {
            XbimMeshFragmentCollection results = new XbimMeshFragmentCollection();
            results.AddRange(this.Where(frag => !(typeof(T).IsAssignableFrom(frag.EntityType))));
            return results;
        }

        /// <summary>
        /// returns the mesh fragment that contains the specified vertex index
        /// an empty fragment is returned if one is not found
        /// uses binary search for speed
        /// </summary>
        /// <param name="vertexIndex"></param>
        /// <returns></returns>
        public XbimMeshFragment Find(int vertexIndex)
        {
            int found = this.BinarySearch(new XbimMeshFragment(vertexIndex,0, 0), new XbimMeshComparer());
            if(found >=0)
                return this[found];
            else
                return default(XbimMeshFragment);
        }
    }
}
