using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace Xbim.ModelGeometry.Scene
{
   
    public class XbimMeshLayerCollection<TVISIBLE, TMATERIAL> : KeyedCollection<string, XbimMeshLayer<TVISIBLE, TMATERIAL>>, INotifyCollectionChanged, INotifyPropertyChanged
        where TVISIBLE : IXbimMeshGeometry3D, new()
        where TMATERIAL : IXbimRenderMaterial, new()
    {

        public XbimMeshLayerCollection()
        {
            
        }

        protected override string GetKeyForItem(XbimMeshLayer<TVISIBLE, TMATERIAL> item)
        {
            return item.Name;
        }

        private event NotifyCollectionChangedEventHandler _collectionChanged;
        public event PropertyChangedEventHandler PropertyChanged;
        private static PropertyChangedEventArgs countPropChangedEventArgs = new PropertyChangedEventArgs("Count");

        public event NotifyCollectionChangedEventHandler CollectionChanged
        {
            add { _collectionChanged += value; }
            remove { _collectionChanged -= value; }
        }

        protected override void InsertItem(int index, XbimMeshLayer<TVISIBLE, TMATERIAL> item)
        {

            XbimMeshLayer<TVISIBLE, TMATERIAL> removed = null;
            if (index < Count)
                removed = this[index];
            base.InsertItem(index, item);
            NotifyCollectionChangedEventHandler collChanged = _collectionChanged;
            if (collChanged != null)
            {
                if (index == Count)
                    collChanged(this,
                                new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, removed, index));
                else
                {
                    collChanged(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item));
                    NotifyCountChanged(Count - 1);
                }
            }
        }

        void SubLayersChanged(object sender, NotifyCollectionChangedEventArgs e) //throw sublayer messages upwards
        {
            NotifyCollectionChangedEventHandler collChanged = _collectionChanged;
            if (collChanged != null)
            {
                collChanged(this, e);
            }
        }

        protected override void RemoveItem(int index)
        {

            int oldCount = Count;
            XbimMeshLayer<TVISIBLE, TMATERIAL> removed = this[index];
            base.RemoveItem(index); 
            NotifyCollectionChangedEventHandler collChanged = _collectionChanged;
            if (collChanged != null)
                collChanged(this,
                            new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, removed, index));
            NotifyCountChanged(oldCount);
        }

        protected override void ClearItems()
        {
            int oldCount = Count;
            base.ClearItems();
            NotifyCollectionChangedEventHandler collChanged = _collectionChanged;
            if (collChanged != null)
                collChanged(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
            NotifyCountChanged(oldCount);
        }

        protected override void SetItem(int index, XbimMeshLayer<TVISIBLE, TMATERIAL> item)
        {
            XbimMeshLayer<TVISIBLE, TMATERIAL> removed = null;
            if (index < Count)
                removed = this[index];
            base.SetItem(index, item);
            NotifyCollectionChangedEventHandler collChanged = _collectionChanged;
            if (collChanged != null)
                collChanged(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, removed, index));
        }

        

        private void NotifyCountChanged(int oldValue)
        {

            if (PropertyChanged != null && oldValue != Count)
                PropertyChanged(this, countPropChangedEventArgs);
        }

        

    }
}
