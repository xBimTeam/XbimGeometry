using System;

namespace Xbim.Tessellator.MeshSimplification
{
    public class MinHeap
    {
        private HeapNode[] _nodeTree;
        private int _numNodes;
        private int[] _idIndexMap;

        public MinHeap(int initialSize)
        {
            _nodeTree = new HeapNode[1];
            _numNodes = 0;

            _idIndexMap = new int[initialSize];
            _nodeTree = new HeapNode[initialSize];
            for (var i = 0; i < _idIndexMap.Length; i++)
                _idIndexMap[i] = -1;
        }

        public int Count => _numNodes;

        public bool Contains(int id)
        {
            if (id < 0 || id >= _idIndexMap.Length)
                return false;
            var idx = _idIndexMap[id];
            return idx > 0 && idx <= _numNodes;
        }

        public void Push(int id, float priority)
        {
            if (Contains(id))
                throw new InvalidOperationException($"IndexPriorityQueue.Insert: id {id} already exists in queue.");

            EnsureIdMapCapacity(id);
            _numNodes++;
            EnsureHeapCapacity(_numNodes);

            var node = new HeapNode { Id = id, Priority = priority, Index = _numNodes };
            _nodeTree[_numNodes] = node;
            _idIndexMap[id] = _numNodes;

            SiftUp(_numNodes);
        }

        public int PopMin()
        {
            if (_numNodes == 0)
                throw new InvalidOperationException("IndexPriorityQueue.Dequeue: queue is empty!");

            var result = _nodeTree[1].Id;
            RemoveAtIndex(1);
            return result;
        }

        public void Update(int id, float priority)
        {
            if (!Contains(id))
                throw new InvalidOperationException($"IndexPriorityQueue.Update: id {id} not found in queue.");

            var iNode = _idIndexMap[id];
            ref var n = ref _nodeTree[iNode];
            n.Priority = priority;

            OnNodeUpdated(iNode);
        }

        private void EnsureIdMapCapacity(int id)
        {
            if (id < _idIndexMap.Length)
                return;
            var newSize = Math.Max(_idIndexMap.Length * 2, id + 1);
            var oldSize = _idIndexMap.Length;
            Array.Resize(ref _idIndexMap, newSize);
            for (var i = oldSize; i < newSize; i++)
                _idIndexMap[i] = -1;
        }

        private void EnsureHeapCapacity(int needed)
        {
            if (needed < _nodeTree.Length)
                return;
            var newSize = Math.Max(_nodeTree.Length * 2, needed + 1);
            Array.Resize(ref _nodeTree, newSize);
        }

        private void RemoveAtIndex(int iNode)
        {
            if (iNode == _numNodes)
            {
                _nodeTree[iNode] = default;
                _numNodes--;
                return;
            }

            SwapNodes(iNode, _numNodes);
            _nodeTree[_numNodes] = default;
            _numNodes--;
            OnNodeUpdated(iNode);
        }

        private void SwapNodes(int i1, int i2)
        {
            ref var n1 = ref _nodeTree[i1];
            ref var n2 = ref _nodeTree[i2];

            n1.Index = i2;
            n2.Index = i1;

            (n1, n2) = (n2, n1);

            _idIndexMap[n1.Id] = i1;
            _idIndexMap[n2.Id] = i2;
        }

        private void SiftUp(int iNode)
        {
            var start = iNode;
            var startNode = _nodeTree[start];
            var parent = start / 2;

            while (parent >= 1 && _nodeTree[parent].Priority > startNode.Priority)
            {
                Move(parent, iNode);
                iNode = parent;
                parent = iNode / 2;
            }

            if (iNode != start)
                Set(iNode, ref startNode);
        }

        private void SiftDown(int iNode)
        {
            var start = iNode;
            var startNode = _nodeTree[start];

            while (true)
            {
                var moveTo = iNode;
                var left = 2 * iNode;
                if (left > _numNodes) break;

                var best = startNode.Priority;
                if (_nodeTree[left].Priority < best)
                {
                    best = _nodeTree[left].Priority;
                    moveTo = left;
                }
                var right = left + 1;
                if (right <= _numNodes && _nodeTree[right].Priority < best)
                    moveTo = right;

                if (moveTo == iNode) break;

                Move(moveTo, iNode);
                iNode = moveTo;
            }

            if (iNode != start)
                Set(iNode, ref startNode);
        }

        private void Move(int from, int to)
        {
            ref var n = ref _nodeTree[from];
            n.Index = to;
            _nodeTree[to] = n;
            _idIndexMap[n.Id] = to;
        }

        private void Set(int to, ref HeapNode n)
        {
            n.Index = to;
            _nodeTree[to] = n;
            _idIndexMap[n.Id] = to;
        }

        private void OnNodeUpdated(int iNode)
        {
            var parent = iNode / 2;
            if (parent > 0 && ShouldBubbleUp(iNode, parent))
                SiftUp(iNode);
            else
                SiftDown(iNode);
        }

        private bool ShouldBubbleUp(int higher, int lower)
        {
            return _nodeTree[higher].Priority < _nodeTree[lower].Priority;
        }

        private struct HeapNode
        {
            public int Id;
            public float Priority;
            public int Index;
        }
    }
}
