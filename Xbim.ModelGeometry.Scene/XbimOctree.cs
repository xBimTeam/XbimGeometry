//This implementation of octree is modification of the 
//code from Ploobs Engine http://ploobs.com.br/?p=1622
//
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// A class to cluster spatial items in iteratively narrower space boundaries.
    /// Formerly used in XbimMesher to split large models like this:
    /// 
    /// XbimOctree octree = new XbimOctree(bounds.Length(), MaxWorldSize * metre, 1f, bounds.Centroid());
    /// octree.Add(geomData.GeometryLabel, bound);
    /// then iterating over octree.Populated to retrieve the clusters.
    /// 
    /// Warning: If items fall across boundaries of children they stop the iterative split of the parent they fall into.
    /// </summary>
    /// <typeparam name="T">e.g. Int for geometry labels</typeparam>
    public class XbimOctree<T>
    {
        public String Name = "R"; // for Root

        /// <summary>
        /// The number of children in an octree.
        /// </summary>
        private const int ChildCount = 8;

        /// <summary>
        /// The octree's looseness value.
        /// </summary> 
        private double looseness = 0;

        /// <summary>
        /// The octree's depth.
        /// </summary> 
        private int depth = 0;

        /// <summary> 
        /// The octree's centre coordinates.
        /// </summary> 
        private XbimPoint3D centre = XbimPoint3D.Zero;


        /// <summary>
        /// The octree's length.
        /// </summary>
        private double length = 0f;

        ///  <summary>
        /// The bounding box that represents the octree.
        /// </summary> 
        private XbimRect3D bounds = default(XbimRect3D);

        ///  <summary>
        /// The objects in the octree.
        /// </summary> 

        private List<T> objects = new List<T>();

        /// <summary>
        /// The octree's child nodes.
        /// </summary> 
        private XbimOctree<T>[] children = null;

        /// <summary>
        /// Parent of this subtree or null if this is the root node
        /// </summary>
        private XbimOctree<T> parent = null;

        /// <summary>
        /// Local address of this node (like [1,1,1]), null for the root node
        /// </summary>
        private OctAddress localAddress = null;

        ///  <summary>
        /// The octree's world size.
        /// </summary> 
        private double worldSize = 0f;
        private double targetCanvasSize = 100000.0;

        private XbimRect3D contentBounds = XbimRect3D.Empty;
        
        /// <summary>
        /// Creates a new octree.
        /// </summary>
        /// <param name="worldSize">/// The octree's world size.</param>
        /// <param name="targetCanvasSize">The octree recursion depth.</param>
        /// <param name="looseness">The octree's looseness value.</param>
        public XbimOctree(double worldSize, double targetCanvasSize, double looseness)
            : this(worldSize, targetCanvasSize, looseness, 0, XbimPoint3D.Zero)
        {
        }
        public XbimOctree(double worldSize, double targetCanvasSize, double looseness, XbimPoint3D centre)
            : this(worldSize, targetCanvasSize, looseness, 0, centre)
        {
        }
        

        /// <summary>
        /// Creates a new octree.
        /// </summary>
        /// <param name="worldSize">The octree's world size.</param>
        /// <param name="targetCanvasSize"></param>
        /// <param name="looseness">The octree's looseness value.</param>
        /// <param name="depth">The maximum depth to recurse to.</param>
        /// <param name="centre">The octree's centre coordinates.</param>
        private XbimOctree(double worldSize, double targetCanvasSize, double looseness, int depth, XbimPoint3D centre)
        {
            this.worldSize = worldSize;
            this.targetCanvasSize = targetCanvasSize;
            this.looseness = looseness;
            this.depth = depth;
            this.centre = centre;
            this.length = this.looseness * this.worldSize / Math.Pow(2, this.depth);
            double radius = this.length / 2f;

            // Create the bounding box.
            XbimPoint3D min = this.centre + new XbimVector3D(-radius);
            XbimPoint3D max = this.centre + new XbimVector3D(radius);
            this.bounds = new XbimRect3D(min, max);

            ////// Split the octree if the depth hasn't been reached.
            //if (this.depth < maxDepth)
            //{
            //    this.Split(maxDepth);
            //}
        }

        public XbimOctree(XbimOctree<T> copy)
        {
            worldSize = copy.worldSize;
            length = copy.length;
            objects = copy.objects;
            children = copy.children;
            centre = copy.centre;
            bounds = copy.bounds;
        }

        public XbimRect3D Bounds { get 
        { 
            return new XbimRect3D() 
            { 
                X = bounds.X, 
                Y = bounds.Y, 
                Z = bounds.Z, 
                SizeX = bounds.SizeX, 
                SizeY = bounds.SizeY, 
                SizeZ = bounds.SizeZ
            }; } 
        }

        public IEnumerable<XbimOctree<T>> Children { get 
        {
            if (children == null) return new XbimOctree<T>[] { };
            return children;
        } }

        public int Depth { get { return depth; } }

        /// <summary>
        /// Returns the main octrees that are populated
        /// </summary>
        public List<XbimOctree<T>> Populated
        {
            get 
            {   List<XbimOctree<T>> population = new List<XbimOctree<T>>();
                return GetPopulation(population);
            }
        }
        /// <summary>
        /// Returns the total bounds for all contetn under this node
        /// </summary>
        /// <returns></returns>
        public XbimRect3D ContentBounds()
        {
            XbimRect3D b = contentBounds;
            if (children != null)
            {
                foreach (var child in children)
                {
                    XbimRect3D cb = child.ContentBounds();
                    if (!cb.IsEmpty) b.Union(cb);
                }
            }
            return b;
        }

        
        private List<XbimOctree<T>> GetPopulation(List<XbimOctree<T>> population)
        {
            // If anything has been added at any level of the tree all children are ignored
            // this can easily happen if an object added is sitting across children and not completely within.
            // todo: investigate looseness 
            if (this.objects != null && this.objects.Any())
                population.Add(this);
            else if (children != null)
            {
                foreach (var child in children)
                {
                    child.GetPopulation(population);
                }
            }
            return population;
        }
        
        /// <summary>
        /// Removes the specified obj.
        /// </summary>
        /// <param name="obj">the object to remove.</param>
        public void Remove(T obj)
        {
            objects.Remove(obj);
        }

        /// <summary>
        /// Determines whether the specified obj has changed.
        /// </summary>
        /// <param name="obj"></param>
        /// <param name="bBox"></param>
        /// <returns>true if the specified obj has changed; otherwise, false.</returns>
        public bool HasChanged(T obj, XbimRect3D bBox)
        {
            return this.bounds.Contains(bBox);
        }

       
        /// <summary>
        /// Adds the given object to the octree.
        /// </summary>
        /// <param name="o">The object to add.</param>
        /// <param name="centre">The object's centre coordinates.</param>
        /// <param name="radius">The object's radius.</param>
        private XbimOctree<T> Add(T o, XbimPoint3D centre, double radius)
        {
            XbimPoint3D min = centre - new XbimVector3D(radius);
            XbimPoint3D max = centre + new XbimVector3D(radius);
            XbimRect3D bounds = new XbimRect3D(min, max);

            if (this.bounds.Contains(bounds))
            {
                return this.Add(o, bounds, centre, radius);
            }
            return null;
        }


        /// <summary>
        /// Adds the given object to the octree.
        /// </summary>
        /// <param name="o"></param>
        /// <param name="bBox"></param>
        public XbimOctree<T> Add(T o, XbimRect3D bBox)
        {
            double radius = bBox.Radius();
            XbimPoint3D centre = bBox.Centroid();
            if (this.bounds.Contains(bBox))
            {
                return this.Add(o, bBox, centre, radius);
            }
            return null;
        }

        /// <summary>
        /// Adds the given object to the octree.
        /// </summary>
        /// <param name="o">The object to add.</param>
        /// <param name="b">The object's bounds.</param>
        /// <param name="centre">The object's centre coordinates.</param>
        /// <param name="radius">The object's radius.</param>
        /// <returns></returns>
        private XbimOctree<T> Add(T o, XbimRect3D b, XbimPoint3D centre, double radius)
        {
            lock (this.objects)
            {
                if (this.children == null && this.bounds.Length() > this.targetCanvasSize)
                    this.Split();
            }
            if (this.children != null)
            {
                // Find which child the object is closest to based on where the
                // object's centre is located in relation to the octree's centre.
                int index = (centre.X <= this.centre.X ? 0 : 1) +
                    (centre.Y >= this.centre.Y ? 0 : 4) +
                    (centre.Z <= this.centre.Z ? 0 : 2);

                // Add the object to the child if it is fully contained within
                // it.
                if (this.children[index].bounds.Contains(b))
                {
                    return this.children[index].Add(o, b, centre, radius);
                }
            }
            // Debug.WriteLine("Addedto: " + this.Name);
            this.objects.Add(o); //otherwise add it to here
            if (contentBounds.IsEmpty) contentBounds = b;
            else contentBounds.Union(b);
            return this;
        }
        /// <summary>
        /// Returns the total content of this octree and all its children (recursive in the tree structure)
        /// This was wrong implementation (not recursive but only for 2 levels) until 20/12/2013. 
        /// Fixed by Martin Cerny
        /// </summary>
        /// <returns>Overall content of the tree node and all subtrees</returns>
        public IEnumerable<T> ContentIncludingChildContent()
        {
            foreach (var o in objects)
            {
                yield return o;
            }
            if (children != null)
            {
                foreach (var child in children)
                {
                    foreach (var co in child.ContentIncludingChildContent())
                    {
                        yield return co;
                    }
                }
            }
        }

        /// <summary>
        /// Returns the content of this node and content of all it't parents but only in the upper direction.
        /// </summary>
        /// <returns>Returns the content of this node and content of all it't parents.</returns>
        public IEnumerable<T> ContentIncludingParentContent()
        {
            foreach (var o in objects)
            {
                yield return o;
            }

            if (parent != null)
            {
                foreach (var item in parent.ContentIncludingParentContent())
                {
                    yield return item;
                }
            }
        }

        public IEnumerable<T> Content()
        {
            foreach (var item in objects)
                yield return item;
        }
       
        /// <summary>
        /// Splits the octree into eight children.
        /// </summary>
        private void Split()
        {
            this.children = new XbimOctree<T>[XbimOctree<T>.ChildCount];
            int depth = this.depth + 1;
            double quarter = this.length / this.looseness / 4f;

            this.children[0] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(-quarter, quarter, -quarter)) { localAddress = new OctAddress( new int[]{ -1, 1, -1 } ) };
            this.children[1] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(quarter, quarter, -quarter)) { localAddress = new OctAddress( new int[] { 1, 1, -1 } ) };
            this.children[2] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(-quarter, quarter, quarter)) { localAddress = new OctAddress( new int[] { -1, 1, 1 } ) };
            this.children[3] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(quarter, quarter, quarter)) { localAddress = new OctAddress( new int[] { 1, 1, 1 } ) };
            this.children[4] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(-quarter, -quarter, -quarter)) { localAddress = new OctAddress( new int[] { -1, -1, -1 } ) };
            this.children[5] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(quarter, -quarter, -quarter)) { localAddress = new OctAddress( new int[] { 1, -1, -1 } ) };
            this.children[6] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(-quarter, -quarter, quarter)) { localAddress = new OctAddress( new int[] { -1, -1, 1 } ) };
            this.children[7] = new XbimOctree<T>(this.worldSize, this.targetCanvasSize, this.looseness,
                 depth, this.centre + new XbimVector3D(quarter, -quarter, quarter)) { localAddress = new OctAddress( new int[] { 1, -1, 1 } ) };
            for (int i = 0; i < children.Length; i++)
            {
                children[i].Name = this.Name + i.ToString();
                children[i].parent = this;
            }
        }

        /// <summary>
        /// Function for search in the tree
        /// </summary>
        /// <param name="item">Object to be found</param>
        /// <returns>Tree or subtree which contains the item as it's own content (not just in the subtree)</returns>
        public XbimOctree<T> Find(T item)
        {
            //if this node contains the item return this
            if (objects.Contains(item))
                return this;
            
            //try to find the item in subtrees
            if (children != null)
            {
                foreach (var child in children)
                {
                    //recursive call on the deeper level
                    var res = child.Find(item);
                    if (res != null) 
                        return res;
                }
            }

            //return null if there is none
            return null;
        }

        /// <summary>
        /// This will get the neighbour cells on the same level of resolution
        /// </summary>
        /// <param name="direction">Direction where to look for the neighbour</param>
        /// <param name="onlySameResolution">If this is true and node of the same 
        /// resolution doesn't exist as a neighbour NULL will be returned</param>
        /// <returns>neighbouring cell on the same or lower level of resolution or null.</returns>
        public XbimOctree<T> GetNeighbour(XbimDirectionEnum direction, bool onlySameResolution)
        {
            //if there is no parent there are no neighbours
            if (parent == null) 
                return null;
            int[] directionCode = new int[] { 0, 0, 0 };
            switch (direction)
            {
                case XbimDirectionEnum.WEST:
                    directionCode = new int[] { -1, 0, 0 };
                    break;
                case XbimDirectionEnum.EAST:
                    directionCode = new int[] { 1, 0, 0 };
                    break;
                case XbimDirectionEnum.NORTH:
                    directionCode = new int[] { 0, 1, 0 };
                    break;
                case XbimDirectionEnum.SOUTH:
                    directionCode = new int[] { 0, -1, 0 };
                    break;
                case XbimDirectionEnum.UP:
                    directionCode = new int[] { 0, 0, 1 };
                    break;
                case XbimDirectionEnum.DOWN:
                    directionCode = new int[] { 0, 0, -1 };
                    break;
                default:
                    break;
            }

            if (localAddress.IsStillIn(directionCode))
            {
                //moved address is still within the same parent
                return parent.GetChild(localAddress.Move(directionCode));
            }
            else
            {
                //this is recursive call drilling up in the tree to get neighbour from the next branch
                var masterNeighbour = parent.GetNeighbour(direction, onlySameResolution);
                if (masterNeighbour != null)
                    //if there is master neighbour but there is not a child it just means that there is 
                    //nothing in the same level of resolution. But higher resolution shouldn't be ignored.
                    return 
                        masterNeighbour.GetChild(localAddress.Move(directionCode)) ?? 
                        masterNeighbour;
            }

            return null;
        }

        public XbimOctree<T> GetCommonParentInDirection(XbimDirectionEnum direction, bool controlRange)
        {
            //if there is no parent there are no neighbours
            if (parent == null)
                return null;
            int[] directionCode = new int[] { 0, 0, 0 };
            switch (direction)
            {
                case XbimDirectionEnum.WEST:
                    directionCode = new int[] { -1, 0, 0 };
                    break;
                case XbimDirectionEnum.EAST:
                    directionCode = new int[] { 1, 0, 0 };
                    break;
                case XbimDirectionEnum.NORTH:
                    directionCode = new int[] { 0, 1, 0 };
                    break;
                case XbimDirectionEnum.SOUTH:
                    directionCode = new int[] { 0, -1, 0 };
                    break;
                case XbimDirectionEnum.UP:
                    directionCode = new int[] { 0, 0, 1 };
                    break;
                case XbimDirectionEnum.DOWN:
                    directionCode = new int[] { 0, 0, -1 };
                    break;
                default:
                    break;
            }

            if (localAddress.IsStillIn(directionCode))
                //moved address is still within the same parent
                return parent;
            else if (parent != null)
            {
                //this is recursive call drilling up in the tree
                return parent.GetCommonParentInDirection(direction, controlRange);
            }

            if (controlRange)
                throw new IndexOutOfRangeException("Direction goes out of the tree world.");
            else
                return null;
        }

        /// <summary>
        /// Returns all 8 subtrees or empty set if this is a leaf
        /// </summary>
        public IEnumerable<XbimOctree<T>> Subtrees
        {
            get 
            {
                return children ?? new XbimOctree<T>[0];
            }
        }

        /// <summary>
        /// Returns parent node or null for the root
        /// </summary>
        public XbimOctree<T> Parent
        {
            get
            {
                return parent;
            }
        }

        private XbimOctree<T> GetChild(OctAddress address)
        {
            return this[address[0], address[1], address[2]];
        }

        private XbimOctree<T> this[int x, int y, int z]
        {
            get
            {
                if (children == null || x == 0 || y == 0 || z == 0)
                    return null;
                if (x > 0)
                {
                    if (y > 0)
                    {
                        if (z > 0)
                            return children[4];
                        else
                            return children[2];
                    }
                    else
                    {
                        if (z > 0)
                            return children[8];
                        else
                            return children[6];
                    }
                }
                else
                {
                    if (y > 0)
                    {
                        if (z > 0)
                            return children[3];
                        else
                            return children[1];
                    }
                    else
                    {
                        if (z > 0)
                            return children[7];
                        else
                            return children[5];
                    }
                }
            }
            
        }
    }

    internal class OctAddress
    {
        private int[] _address;

        public OctAddress(int x, int y, int z)
        {
            _address = new int[] { x, y, z };
            CheckAddress(_address);
        }
        public OctAddress(int[] address)
        {
            _address = address;
            CheckAddress(_address);
        }

        /// <summary>
        /// This will return new address moved to the new position. 
        /// Result is always in the correct format +1/-1
        /// </summary>
        /// <param name="x">move directions +1/-1/0</param>
        /// <param name="y">move directions +1/-1/0</param>
        /// <param name="z">move directions +1/-1/0</param>
        /// <returns>New address</returns>
        public OctAddress Move(int x, int y, int z)
        {
            return Move(new[] { x, y, z });
        }

        /// <summary>
        /// This will return new address moved to the new position. 
        /// Result is always in the correct format +1/-1
        /// </summary>
        /// <param name="move">move directions +1/-1/0</param>
        /// <returns>New address</returns>
        public OctAddress Move(int[] move)
        {
            CheckMove(move);
            var result = new int[3];
            for (int i = 0; i < 3; i++)
            {
                result[i] = Addition(_address[i], move[i]);
            }
            return new OctAddress(result);
        }

        public bool IsStillIn(int[] move)
        {
            CheckMove(move);
            for (int i = 0; i < 3; i++)
                if (Math.Abs(_address[i] + move[i]) > 1) return false;
            return true;
        }

        /// <summary>
        /// Result will allways stay in the bounds of the address
        /// </summary>
        /// <param name="address"></param>
        /// <param name="move"></param>
        /// <returns></returns>
        private int Addition(int address, int move)
        {
            var res = address+move;
            if (res == 1 || res == -1) return res;
            //reverse addres to the other side
            if (res == 0 || Math.Abs(res) > 1) return -address;

            throw new NotImplementedException("Unexpected argument or execution path.");
        }

        //numbers and operations restricted to +1, -1
        private void CheckAddress(int[] i)
        {
            if (i.Length != 3)
                throw new Exception("This is not legal format of the address.");
            foreach (var item in i)
                if (item != 1 && item != -1) throw new Exception("This is not legal format of the address.");
        }

        //numbers and operations restricted to +1, -1
        private void CheckMove(int[] i)
        {
            if (i.Length != 3)
                throw new Exception("This is not legal format of the move.");
            foreach (var item in i)
                if (item != 1 && item != -1 && item != 0) throw new Exception("This is not legal format of the move.");
        }

        public int this[int i]
        {
            get { return _address[i]; }
        }
    }
}
