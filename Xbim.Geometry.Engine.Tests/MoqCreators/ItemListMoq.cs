using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;
using Xbim.Common;

namespace Xbim.Geometry.Engine.Tests
{
    public class ItemListMoq<T> : IItemSet<T>
    {
        List<T> impl = new List<T>();
        public T this[int index] { get => impl[index]; }
        T IList<T>.this[int index] { get => impl[index]; set => impl[index] = value; }

        public int Count =>
            impl.Count;

        public bool IsReadOnly => false;

        public IPersistEntity? OwningEntity => null;

        public event NotifyCollectionChangedEventHandler? CollectionChanged;
        public event PropertyChangedEventHandler? PropertyChanged;

        protected void OnCollectionChanged()
        {
            if (CollectionChanged != null)
                CollectionChanged(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }
        protected void OnPropertyChanged()
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs("Count"));
        }
        public void Add(T item)
        {
            impl.Add(item);
        }

        public void AddRange(IEnumerable<T> values)
        {
            impl.AddRange(values);
        }

        public void Clear()
        {
            impl.Clear();
        }

        public bool Contains(T item)
        {
            return impl.Contains(item);
        }

        public void CopyTo(T[] array, int arrayIndex)
        {
            impl.CopyTo(array, arrayIndex);
        }

        public T FirstOrDefault(Func<T, bool> predicate)
        {
#pragma warning disable CS8603 // Possible null reference return.
            return impl.FirstOrDefault();
#pragma warning restore CS8603 // Possible null reference return.
        }

        public TF FirstOrDefault<TF>(Func<TF, bool> predicate) where TF : T
        {
#pragma warning disable CS8600 // Converting null literal or possible null value to non-nullable type.
            var e = (TF)impl.FirstOrDefault();
#pragma warning restore CS8600 // Converting null literal or possible null value to non-nullable type.
#pragma warning disable CS8604 // Possible null reference argument.
#pragma warning disable CS8603 // Possible null reference return.
            if (predicate(e))
                return e;
            else return default;
#pragma warning restore CS8603 // Possible null reference return.
#pragma warning restore CS8604 // Possible null reference argument.
        }

        public T GetAt(int index)
        {
            return impl[index];
        }

        public IEnumerator<T> GetEnumerator()
        {
            return impl.GetEnumerator();
        }

        public int IndexOf(T item)
        {
            return impl.IndexOf(item);
        }

        public void Insert(int index, T item)
        {
            impl.Insert(index, item);
        }

        public bool Remove(T item)
        {
            return impl.Remove(item);
        }

        public void RemoveAt(int index)
        {
            impl.RemoveAt(index);
        }

        public IEnumerable<TW> Where<TW>(Func<TW, bool> predicate) where TW : T
        {
            foreach (var item in impl.Cast<TW>())
            {
                if (predicate(item)) yield return item;
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return impl.GetEnumerator();
        }
    }
}
