using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// This class can be used to encode XYZ (arity 3) data or matrices (arity 9 for matrices 3x3 or 16 for matrices 4x4)
    /// into the bitmap image which can than be used
    /// in WebGL applications or elsewhere where image is the best format for data transfer. If data
    /// exceed size of one bitmap new bitmaps are created automatically.
    /// </summary>
    public class XbimFloatEncodedBitmap
    {
        private Bitmap _bmp;
        private int _size;
        private int _arity;

        //current index within actual image
        private long _dataIndex = -1;

        /// <summary>
        /// Constructor will create rectangle bitmap of the desired size 
        /// </summary>
        /// <param name="size">Resulting image will have resolution size x size</param>
        /// <param name="arity">How many floats are in one data entry. This is ie. 3 for points or vec3, 9 for matrices 3x3, 16 for matrices 4x4
        /// or any other number for different things to be stored</param>
        public XbimFloatEncodedBitmap(int size, int arity)
        {
            if (arity < 1)
                throw new ArgumentException("Minimal value of arity is 1.", "arity");
            if (size < 1)
                throw new ArgumentException("Minimal value of size is 1.", "size");

            _arity = arity;
            _size = size;
            _bmp = new Bitmap(_size, _size);
        }

        /// <summary>
        /// Gets actual size of bitmap. This is always number dividable by 3
        /// </summary>
        public int BitmapSize { get { return _size; } }

        /// <summary>
        /// Gets current data entry index. 
        /// </summary>
        public long CurrentIndex { get { return _dataIndex; } set { _dataIndex = value; } }

        /// <summary>
        /// Gets underlying bitmap containing encoded data
        /// </summary>
        public Bitmap Bitmap { get { return _bmp; } }

        /// <summary>
        /// This function will save the bitmap to the specified PNG file. Extension will be added if no extension is specified.
        /// </summary>
        /// <param name="path">Target file path</param>
        // ReSharper disable once InconsistentNaming
        public void SaveAsPNG(string path)
        {
            var extension = Path.GetExtension(path);
            if (extension != null && extension.ToLower() != ".png")
                path += ".png";
            using (var stream = File.Create(path))
            {
                SaveAsPNG(stream);
                stream.Close();
            }
        }

        /// <summary>
        /// This function writes bitmap image to the stream as a PNG image.
        /// This function WON'T close the stream.
        /// </summary>
        /// <param name="stream">Stream used to write the PNG image</param>
        // ReSharper disable once InconsistentNaming
        public void SaveAsPNG(Stream stream)
        {
            _bmp.Save(stream, System.Drawing.Imaging.ImageFormat.Png);
        }

        /// <summary>
        /// Adds data entries to the image and returns index of the last one. Number of floats must be multiple of arity.
        /// </summary>
        /// <param name="dataEntries">Data entries in one single list. If number of floats is not multiple of arity exception is thrown.</param>
        /// <returns>Index of last data entry added to bitmap.</returns>
        public long AddDataEntries(IList<float> dataEntries)
        {
            if (dataEntries.Count % _arity != 0)
                throw new ArgumentOutOfRangeException("dataEntries", "Number of entries must be multiple of arity.");
            var count = dataEntries.Count / _arity;
            var entriesArray = dataEntries.ToArray();
            var oneEntryArray = new float[_arity];

            for (int i = 0; i < count; i++)
            {
                Array.Copy(entriesArray, i * _arity, oneEntryArray, 0, _arity);
                AddDataEntry(oneEntryArray);
            }
            return _dataIndex;
        }

        /// <summary>
        /// Adds data entry to the image and returns index.
        /// </summary>
        /// <param name="values">List of values which count is arity. Any other number will throw an exception.</param>
        /// <returns>Index of the new data entry.</returns>
        public long AddDataEntry(IList<float> values)
        {
            if (values.Count != _arity)
                throw new ArgumentException(string.Format("There must be exactly {0} values specified.", _arity));

            SetDataEntry(values, ++_dataIndex);

            return _dataIndex;
        }

        private int[] GetImgCoords(long index)
        {
            return new int[] 
            {
                (int)index % _size,
                (int)index / _size
            };
        }

        /// <summary>
        /// This function will set defined values in bitmap. If data exceeds current bitmap new bitmap will be alocated.
        /// Number of values must be the same as arity specified in constructor. This function won't reset current index.
        /// If you want to start adding values from some offset set the property of index to that value.
        /// </summary>
        /// <param name="values">Values to be set. Number of values must match arity</param>
        /// <param name="index">Index of the data entry.</param>
        public void SetDataEntry(IList<float> values, long index)
        {
            if (values.Count != _arity)
                throw new ArgumentException(string.Format("There must be exactly {0} values specified.", _arity));

            var pixelIndex = index * _arity; //to get actual index
            var imageLength = _size * _size;

            //check if there is enough space for the data entry so that it is not encoded in two bitmaps
            if (pixelIndex + _arity > imageLength)
                throw new OverflowException("Data entry would overflow size of bitmap. You should have checked available space before.");

            
            //set the pixels to floats representing one data entry
            for (int i = 0; i < _arity; i++)
            {
                var c = GetImgCoords(pixelIndex++);
                var colour = GetColor(values[i]);
                _bmp.SetPixel(c[0], c[1], colour);
            }
        }

        /// <summary>
        /// This function can be used to get data entry created previously with this function.
        /// </summary>
        /// <param name="index">Index of data entry</param>
        /// <returns>Data where length of array is arity defined in constructor</returns>
        public float[] GetDataEntry(long index)
        {
            var result = new float[_arity];
            var pixelIndex = index * _arity; //to get actual index

            
            for (int i = 0; i < _arity; i++)
            {
                var c = GetImgCoords(pixelIndex + i);
                var colour = _bmp.GetPixel(c[0], c[1]);
                var vals = new byte[] 
                {
                    colour.R, colour.G, colour.B, colour.A
                };
                result[i] = DecodeFloat(vals);
            }
            return result;
        }

        /// <summary>
        /// Gets number of data entries which can be written to thi actual bitmap
        /// </summary>
        /// <returns>Number of free data entries</returns>
        public long FreeSpace
        {
            get
            {
                var index = (_dataIndex + 1) * _arity;
                return ((_size * _size) - index) / _arity;
            }
        }

        private Color GetColor(float f)
        {
            byte[] bytes = EncodeFloat(f);
            return Color.FromArgb(bytes[3], bytes[0], bytes[1], bytes[2]);
        }

        private byte[] EncodeFloat(float f)
        {
            var result = BitConverter.GetBytes(f);
            if (BitConverter.IsLittleEndian)
                return result;
            else
                return new byte[] { result[3], result[2], result[1], result[0]};
        }

        private float DecodeFloat(byte[] vals)
        {
            var bits = GetBitArray(vals);

            //split into parts as defined by IEEE
            var sign = bits[0] == 0 ? 1 : -1;
            var exponent = BitsToUInt8(SubArray<int>(bits, 1, 8)) - 127;
            var fraction = GetFraction(SubArray<int>(bits, 9, 23));

            //compute decimal value from IEEE encoding
            var result = sign * fraction * (float)Math.Pow(2, exponent);
            return result;
        }

        private float GetFraction(int[] bits)
        {
            var result = 1f;
            for (var i = 0; i < 23; i++)
            {
                result += bits[i] * (float)Math.Pow(2, (-1)*(i+1));
            }
            return result;
        }

        private int BitsToUInt8(int[] bits)
        {
            var result = 0;
            for (var i = 0; i < 8; i++)
            {
                result += bits[7 - i] * (int)Math.Pow(2, i);
            }
            return result;
        }

        private int[] GetBitArray(byte[] input)
        {
            var result = new int[32];
            for (var i = 0; i < 4; i++)
            {
                var actualByte = input[i];
                for (var j = 0; j < 8; j++)
                {
                    var index = 31 - (j + i * 8);
                    result[index] =  actualByte % 2;
                    actualByte /= 2;
                }
            }
            return result;
        }

        private T[] SubArray<T>(T[] data, int index, int length)
        {
            var result = new T[length];
            Array.Copy(data, index, result, 0, length);
            return result;
        }
    }
}
