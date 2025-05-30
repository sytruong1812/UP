﻿using System;
using System.Text;
using System.Diagnostics;

namespace CloudServer
{
    public class Buffer
    {
        private long _size;
        private long _offset;
        private byte[] _data;

        /// <summary>
        /// Is the buffer empty?
        /// </summary>
        public bool IsEmpty => (_data == null) || (_size == 0);

        /// <summary>
        /// Bytes memory buffer
        /// </summary>
        public byte[] Data => _data;

        /// <summary>
        /// Bytes memory buffer capacity
        /// </summary>
        public long Capacity => _data.Length;

        /// <summary>
        /// Bytes memory buffer size
        /// </summary>
        public long Size => _size;

        /// <summary>
        /// Bytes memory buffer offset
        /// </summary>
        public long Offset => _offset;

        /// <summary>
        /// Buffer indexer operator
        /// </summary>
        public byte this[long index] => _data[index];

        /// <summary>
        /// Initialize a new expandable buffer with zero capacity
        /// </summary>
        public Buffer() { _data = new byte[0]; _size = 0; _offset = 0; }

        /// <summary>
        /// Initialize a new expandable buffer with the given capacity
        /// </summary>
        public Buffer(long capacity) { _data = new byte[capacity]; _size = 0; _offset = 0; }

        /// <summary>
        /// Initialize a new expandable buffer with the given data
        /// </summary>
        public Buffer(byte[] data) { _data = data; _size = data.Length; _offset = 0; }

        #region Memory buffer methods

        /// <summary>
        /// Get a string from the current buffer
        /// </summary>
        public override string ToString()
        {
            return ExtractString(0, _size);
        }

        /// <summary>
        /// Clear the current buffer and its offset
        /// </summary>
        public void Clear()
        {
            _size = 0;
            _offset = 0;
        }

        /// <summary>
        /// Extract the string from buffer of the given offset and size
        /// </summary>
        public string ExtractString(long offset, long size)
        {
            Debug.Assert(((offset + size) <= Size), "Invalid offset & size!");
            if ((offset + size) > Size)
            {
                throw new ArgumentException("Invalid offset & size!", nameof(offset));
            }
            return Encoding.UTF8.GetString(_data, (int)offset, (int)size);
        }

        /// <summary>
        /// Remove the buffer of the given offset and size
        /// </summary>
        public void Remove(long offset, long size)
        {
            Debug.Assert(((offset + size) <= Size), "Invalid offset & size!");
            if ((offset + size) > Size)
            {
                throw new ArgumentException("Invalid offset & size!", nameof(offset));
            }
            Array.Copy(_data, offset + size, _data, offset, _size - size - offset);
            _size -= size;
            if (_offset >= (offset + size))
            {
                _offset -= size;
            }
            else if (_offset >= offset)
            {
                _offset -= _offset - offset;
                if (_offset > Size)
                {
                    _offset = Size;
                }
            }
        }

        /// <summary>
        /// Reserve the buffer of the given capacity
        /// </summary>
        public void Reserve(long capacity)
        {
            Debug.Assert((capacity >= 0), "Invalid reserve capacity!");
            if (capacity < 0)
            {
                throw new ArgumentException("Invalid reserve capacity!", nameof(capacity));
            }   
            if (capacity > Capacity)
            {
                try
                {
                    byte[] data = new byte[Math.Max(capacity, 2 * Capacity)];
                    Array.Copy(_data, 0, data, 0, _size);
                    _data = data;
                }
                catch (OutOfMemoryException ex)
                {
                    throw new OutOfMemoryException("Not enough memory to reserve the requested capacity: " + ex.Message, ex);
                }
            }
        }

        /// <summary>
        /// Resize the current buffer
        /// </summary>
        public void Resize(long size)
        {
            Reserve(size);
            _size = size;
            if (_offset > _size)
            {
                _offset = _size;
            }
        }

        /// <summary>
        /// Shift the current buffer offset
        /// </summary>
        public void Shift(long offset) { _offset += offset; }

        /// <summary>
        /// Unshift the current buffer offset
        /// </summary>
        public void Unshift(long offset) { _offset -= offset; }

        #endregion

        #region Buffer I/O methods

        /// <summary>
        /// Append the single byte
        /// </summary>
        /// <param name="value">Byte value to append</param>
        /// <returns>Count of append bytes</returns>
        public long Append(byte value)
        {
            Reserve(_size + 1);
            _data[_size] = value;
            _size += 1;
            return 1;
        }

        /// <summary>
        /// Append the given buffer
        /// </summary>
        /// <param name="buffer">Buffer to append</param>
        /// <returns>Count of append bytes</returns>
        public long Append(byte[] buffer)
        {
            Reserve(_size + buffer.Length);
            Array.Copy(buffer, 0, _data, _size, buffer.Length);
            _size += buffer.Length;
            return buffer.Length;
        }

        /// <summary>
        /// Append the given buffer fragment
        /// </summary>
        /// <param name="buffer">Buffer to append</param>
        /// <param name="offset">Buffer offset</param>
        /// <param name="size">Buffer size</param>
        /// <returns>Count of append bytes</returns>
        public long Append(byte[] buffer, long offset, long size)
        {
            Reserve(_size + size);
            Array.Copy(buffer, offset, _data, _size, size);
            _size += size;
            return size;
        }

        /// <summary>
        /// Append the given text in UTF-8 encoding
        /// </summary>
        /// <param name="text">Text to append</param>
        /// <returns>Count of append bytes</returns>
        public long Append(string text)
        {
            int length = Encoding.UTF8.GetMaxByteCount(text.Length);
            Reserve(_size + length);
            long result = Encoding.UTF8.GetBytes(text, 0, text.Length, _data, (int)_size);
            _size += result;
            return result;
        }

        /// <summary>
        /// Read the byte from buffer fragment
        /// </summary>
        /// <param name="offset">Buffer offset</param>
        /// <returns>A byte</returns>
        public byte ReadByte(long offset)
        {
            return _data[offset];
        }

        /// <summary>
        /// Read the byte array from buffer fragment
        /// </summary>
        /// <param name="offset">Buffer offset</param>
        /// <param name="size">Buffer size</param>
        /// <returns>A array buffer</returns>
        public byte[] ReadBytes(long offset, long size)
        {
            byte[] buffer = new byte[size];
            Array.Copy(_data, offset, buffer, 0, size);
            return buffer;
        }

        /// <summary>
        /// Read the text from buffer fragment in UTF-8 encoding
        /// </summary>
        /// <param name="offset">Buffer offset</param>
        /// <param name="size">Buffer size</param>
        /// <returns>Text in UTF-8 encoding</returns>
        public string ReadString(long offset, long size)
        {
            byte[] buffer = new byte[size];
            Array.Copy(_data, offset, buffer, 0, size);
            return Encoding.UTF8.GetString(buffer);
        }

        #endregion
    }
}
