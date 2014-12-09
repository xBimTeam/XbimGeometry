using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimRegionCollection : List<XbimRegion>
    {
        #region Serialisation
        new public byte[] ToArray()
        {
            MemoryStream ms = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(ms);
            bw.Write((int)this.Count);
            
            foreach (var region in this)
            {
                bw.Write(region.Name);
                bw.Write(region.Population);
                bw.Write((float)region.Centre.X);
                bw.Write((float)region.Centre.Y);
                bw.Write((float)region.Centre.Z);
                bw.Write((float)region.Size.X);
                bw.Write((float)region.Size.Y);
                bw.Write((float)region.Size.Z);
            }
            bw.Close();
            return ms.ToArray();
        }

        public static XbimRegionCollection FromArray(byte[] bytes)
        {
            XbimRegionCollection coll = new XbimRegionCollection();
            MemoryStream ms = new MemoryStream(bytes);
            BinaryReader br = new BinaryReader(ms);
            int count = br.ReadInt32();
            for (int i = 0; i < count; i++)
            {
                XbimRegion region = new XbimRegion();
                region.Name = br.ReadString();
                region.Population = br.ReadInt32();
                region.Centre.X = br.ReadSingle();
                region.Centre.Y = br.ReadSingle();
                region.Centre.Z = br.ReadSingle();
                region.Size.X = br.ReadSingle();
                region.Size.Y = br.ReadSingle();
                region.Size.Z = br.ReadSingle();
                coll.Add(region);
            }
            return coll;
        }
        #endregion

        public XbimRegion MostPopulated()
        {
            int max = -1;
            XbimRegion mostPopulated = null;
            foreach (var region in this)
            {
                if (region.Population == -1) //indicates everything
                    return region;
                if (region.Population > max)
                {
                    mostPopulated = region;
                    max = region.Population;
                }
            }
            return mostPopulated;
        }

        public XbimRegion Largest()
        {
            double max = 0;
            XbimRegion Largest = null;
            foreach (var region in this)
            {
                if (region.Diagonal() > max)
                {
                    Largest = region;
                    max = region.Diagonal();
                }
            }
            return Largest;
        }
    }
}
