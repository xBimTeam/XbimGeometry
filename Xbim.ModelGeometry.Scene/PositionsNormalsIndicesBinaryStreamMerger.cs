using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using Xbim.Common.Exceptions;
using System.Diagnostics;

namespace Xbim.ModelGeometry.Scene
{
    public class PositionsNormalsIndicesBinaryStreamMerger
    {
        UInt32 iPosNormalOffset = 0;
        public UInt32 iTotPosNormals = 0;
        public UInt32 iTotTriangles = 0;


        MemoryStream msPosNormals = new MemoryStream();
        MemoryStream msIndices = new MemoryStream();

        private byte[] _TransformData = null;
        public byte[] TransformData
        {
            get
            {
                return _TransformData;
            }
        }

        public void Merge(byte[] GeomData, byte[] TransformData)
        {
            PositionsNormalsIndicesBinaryStreamWriter.DebugStream(GeomData, false, "in merge");

            if (_TransformData == null)
            {
                _TransformData = TransformData;
            }
            else
            {
                if (!_TransformData.SequenceEqual(TransformData))
                {
                    throw new XbimException("Geomtries should share the transformArray for this function to work. Different transform still to be implemented.");
                }
            }
            MemoryStream ms = new MemoryStream(GeomData);
            BinaryReader br = new BinaryReader(ms);
            UInt32 numPosNormals = br.ReadUInt32();
            UInt32 numTriangles = br.ReadUInt32();
            iTotPosNormals += numPosNormals;
            iTotTriangles += numTriangles;
            int numBytesPosAndNormals = (int)(numPosNormals * 24); // 24 = 4 bytes (float32) times the 6 coords (3pos + 3 nrm) 
            int numBytesTriangs = (int)(numTriangles * 12); // 3 uint per triangle

            int iPosAndNormalsBegin = 8;

            msPosNormals.Write(GeomData, iPosAndNormalsBegin, numBytesPosAndNormals);
            msPosNormals.Flush();

            if (iPosNormalOffset > 0) // have to increment the indices
            {
                ms.Seek(iPosAndNormalsBegin + numBytesPosAndNormals, SeekOrigin.Begin); // goes where the indices begin
                BinaryWriter IndexWriter = new BinaryWriter(msIndices);
                UInt32 iCnt = 0; // 
                while (iCnt++ < numTriangles * 3)
                {
                    UInt32 OffsetIndex = br.ReadUInt32() + iPosNormalOffset;
                    IndexWriter.Write(OffsetIndex);
                }
            }
            else // write the indices as they are
            {
                msIndices.Write(GeomData, iPosAndNormalsBegin + numBytesPosAndNormals, numBytesTriangs);
            }
            iPosNormalOffset += numPosNormals;
        }

        public void WriteTo(MemoryStream ms)
        {
            BinaryWriter bw = new BinaryWriter(ms);
            bw.Write(iTotPosNormals);
            bw.Write(iTotTriangles);
            bw.Flush();

            msPosNormals.Seek(0, SeekOrigin.Begin);
            msPosNormals.WriteTo(ms);
            ms.Flush();

            msIndices.Seek(0, SeekOrigin.Begin);
            msIndices.WriteTo(ms);
            ms.Flush();
        }
    }
}
