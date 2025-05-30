using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace MultipartFormData
{
	/// <summary>
	///     Provides methods to interpret and read a stream as either character or binary
	///     data similar to a <see cref="BinaryReader" /> and provides the ability to push
	///     data onto the front of the stream.
	/// </summary>
	public class RebufferableBinaryReader
	{
		#region Fields

		/// <summary>
		///     The size of the buffer to use when reading new data.
		/// </summary>
		private readonly int bufferSize;

		/// <summary>
		///     The encoding to use for character based operations.
		/// </summary>
		private readonly Encoding encoding;

		/// <summary>
		///     The stream to read raw data from.
		/// </summary>
		private readonly Stream stream;

		/// <summary>
		///     The stream stack to store buffered data.
		/// </summary>
		private readonly BinaryStreamStack streamStack;

		#endregion

		#region Constructors and Destructors

		/// <summary>
		///     Initializes a new instance of the <see cref="RebufferableBinaryReader" /> class.
		///     Default encoding of UTF8 will be used.
		/// </summary>
		/// <param name="input">
		///     The input stream to read from.
		/// </param>
		public RebufferableBinaryReader(Stream input)
			: this(input, new UTF8Encoding(false))
		{
		}

		/// <summary>
		///     Initializes a new instance of the <see cref="RebufferableBinaryReader" /> class.
		/// </summary>
		/// <param name="input">
		///     The input stream to read from.
		/// </param>
		/// <param name="encoding">
		///     The encoding to use for character based operations.
		/// </param>
		/// <param name="bufferSize">
		///     The buffer size to use for new buffers.
		/// </param>
		public RebufferableBinaryReader(Stream input, Encoding encoding, int bufferSize = 4096)
		{
			stream = input;
			streamStack = new BinaryStreamStack(encoding);
			this.encoding = encoding;
			this.bufferSize = bufferSize;
		}

		#endregion

		#region Public Methods and Operators

		/// <summary>
		///     Adds data to the front of the stream. The most recently buffered data will
		///     be read first.
		/// </summary>
		/// <param name="data">
		///     The data to buffer.
		/// </param>
		public void Buffer(byte[] data)
		{
			streamStack.Push(data);
		}

		/// <summary>
		///     Adds data to the front of the stream. The most recently buffered data will
		///     be read first.
		/// </summary>
		/// <param name="data">
		///     The data to buffer.
		/// </param>
		/// <param name="offset">
		///     The zero-based byte offset in buffer at which to begin copying bytes to the current stream.
		/// </param>
		/// <param name="count">
		///     The maximum number of bytes to write.
		/// </param>
		public void Buffer(byte[] data, int offset, int count)
		{
			streamStack.Push(data, offset, count);
		}

		/// <summary>
		///     Adds the string to the front of the stream. The most recently buffered data will
		///     be read first.
		/// </summary>
		/// <param name="data">
		///     The data.
		/// </param>
		public void Buffer(string data)
		{
			streamStack.Push(encoding.GetBytes(data));
		}

		/// <summary>
		///     Reads a single byte as an integer from the stream. Returns -1 if no
		///     data is left to read.
		/// </summary>
		/// <returns>
		///     The <see cref="byte" /> that was read.
		/// </returns>
		public int Read()
		{
			int value = -1;
			while (value == -1)
			{
				if (!streamStack.HasData())
				{
					if (StreamData() == 0)
					{
						return -1;
					}
				}

				value = streamStack.Read();
			}

			return value;
		}

		/// <summary>
		///     Reads the specified number of bytes from the stream, starting from a
		///     specified point in the byte array.
		/// </summary>
		/// <param name="buffer">
		///     The buffer to read data into.
		/// </param>
		/// <param name="index">
		///     The index of buffer to start reading into.
		/// </param>
		/// <param name="count">
		///     The number of bytes to read into the buffer.
		/// </param>
		/// <returns>
		///     The number of bytes read into buffer. This might be less than the number of bytes requested if that many bytes are not available,
		///     or it might be zero if the end of the stream is reached.
		/// </returns>
		public int Read(byte[] buffer, int index, int count)
		{
			int amountRead = 0;
			while (amountRead < count)
			{
				if (!streamStack.HasData())
				{
					if (StreamData() == 0)
					{
						return amountRead;
					}
				}

				amountRead += streamStack.Read(buffer, index + amountRead, count - amountRead);
			}

			return amountRead;
		}

		/// <summary>
		///     Reads the specified number of characters from the stream, starting from a
		///     specified point in the byte array.
		/// </summary>
		/// <param name="buffer">
		///     The buffer to read data into.
		/// </param>
		/// <param name="index">
		///     The index of buffer to start reading into.
		/// </param>
		/// <param name="count">
		///     The number of characters to read into the buffer.
		/// </param>
		/// <returns>
		///     The number of characters read into buffer. This might be less than the number of
		///     characters requested if that many characters are not available,
		///     or it might be zero if the end of the stream is reached.
		/// </returns>
		public int Read(char[] buffer, int index, int count)
		{
			int amountRead = 0;
			while (amountRead < count)
			{
				if (!streamStack.HasData())
				{
					if (StreamData() == 0)
					{
						return amountRead;
					}
				}

				amountRead += streamStack.Read(buffer, index + amountRead, count - amountRead);
			}

			return amountRead;
		}

		/// <summary>
		///     Reads a series of bytes delimited by the byte encoding of newline for this platform.
		///     The newline bytes will not be included in the return data.
		/// </summary>
		/// <returns>
		///     A byte array containing all the data up to but not including the next newline in the stack.
		/// </returns>
        public byte[] ReadByteLine()
        {
            // Create a new MemoryStream directly
            using (var builder = new MemoryStream())
            {
                while (true)
                {
                    if (!streamStack.HasData())
                    {
                        if (StreamData() == 0)
                        {
                            return builder.Length > 0 ? builder.ToArray() : null;
                        }
                    }

                    byte[] line = streamStack.ReadByteLine(out bool hitStreamEnd);

                    builder.Write(line, 0, line.Length);
                    if (!hitStreamEnd)
                    {
                        return builder.ToArray();
                    }
                }
            }
        }


        /// <summary>
        ///     Reads a line from the stack delimited by the newline for this platform. The newline
        ///     characters will not be included in the stream.
        /// </summary>
        /// <returns>
        ///     The <see cref="string" /> containing the line or null if end of stream.
        /// </returns>
        public string ReadLine()
		{
			byte[] data = ReadByteLine();
			return data == null ? null : encoding.GetString(data);
		}

		/// <summary>
		///     Asynchronously reads a single byte as an integer from the stream.
		///     Returns -1 if no data is left to read.
		/// </summary>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The <see cref="byte" /> that was read.
		/// </returns>
		public async Task<int> ReadAsync(CancellationToken cancellationToken = default)
		{
			int value = -1;
			while (value == -1)
			{
				if (!streamStack.HasData())
				{
					var bytesRead = await StreamDataAsync(cancellationToken).ConfigureAwait(false);
					if (bytesRead == 0)
					{
						return -1;
					}
				}

				value = streamStack.Read();
			}

			return value;
		}

		/// <summary>
		///     Asynchronously reads the specified number of bytes from the stream, starting from a
		///     specified point in the byte array.
		/// </summary>
		/// <param name="buffer">
		///     The buffer to read data into.
		/// </param>
		/// <param name="index">
		///     The index of buffer to start reading into.
		/// </param>
		/// <param name="count">
		///     The number of bytes to read into the buffer.
		/// </param>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The number of bytes read into buffer. This might be less than the number of bytes requested if that many bytes are not available,
		///     or it might be zero if the end of the stream is reached.
		/// </returns>
		public async Task<int> ReadAsync(byte[] buffer, int index, int count, CancellationToken cancellationToken = default)
		{
			int amountRead = 0;
			while (amountRead < count)
			{
				if (!streamStack.HasData())
				{
					var bytesRead = await StreamDataAsync(cancellationToken).ConfigureAwait(false);
					if (bytesRead == 0)
					{
						return amountRead;
					}
				}

				amountRead += streamStack.Read(buffer, index + amountRead, count - amountRead);
			}

			return amountRead;
		}

		/// <summary>
		///     Asynchronously reads the specified number of characters from the stream, starting from a
		///     specified point in the byte array.
		/// </summary>
		/// <param name="buffer">
		///     The buffer to read data into.
		/// </param>
		/// <param name="index">
		///     The index of buffer to start reading into.
		/// </param>
		/// <param name="count">
		///     The number of characters to read into the buffer.
		/// </param>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The number of characters read into buffer. This might be less than the number of
		///     characters requested if that many characters are not available,
		///     or it might be zero if the end of the stream is reached.
		/// </returns>
		public async Task<int> ReadAsync(char[] buffer, int index, int count, CancellationToken cancellationToken = default)
		{
			int amountRead = 0;
			while (amountRead < count)
			{
				if (!streamStack.HasData())
				{
					var bytesRead = await StreamDataAsync(cancellationToken).ConfigureAwait(false);
					if (bytesRead == 0)
					{
						return amountRead;
					}
				}

				amountRead += streamStack.Read(buffer, index + amountRead, count - amountRead);
			}

			return amountRead;
		}

		/// <summary>
		///     Asynchronously reads a series of bytes delimited by the byte encoding of newline for this platform.
		///     The newline bytes will not be included in the return data.
		/// </summary>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     A byte array containing all the data up to but not including the next newline in the stack.
		/// </returns>
		public async Task<byte[]> ReadByteLineAsync(CancellationToken cancellationToken = default)
		{
			using (var builder = new MemoryStream())
			{
				while (true)
				{
					if (!streamStack.HasData())
					{
						var bytesRead = await StreamDataAsync(cancellationToken).ConfigureAwait(false);
						if (bytesRead == 0)
						{
							return builder.Length > 0 ? builder.ToArray() : null;
						}
					}

					byte[] line = streamStack.ReadByteLine(out bool hitStreamEnd);

					await builder.WriteAsync(line, 0, line.Length, cancellationToken).ConfigureAwait(false);
					if (!hitStreamEnd)
					{
						return builder.ToArray();
					}
				}
			}
		}

		/// <summary>
		///     Asynchronously reads a line from the stack delimited by the newline for this platform. The newline
		///     characters will not be included in the stream.
		/// </summary>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The <see cref="string" /> containing the line or null if end of stream.
		/// </returns>
		public async Task<string> ReadLineAsync(CancellationToken cancellationToken = default)
		{
			byte[] data = await ReadByteLineAsync(cancellationToken).ConfigureAwait(false);
			return data == null ? null : encoding.GetString(data);
		}

		#endregion

		#region Methods

		/// <summary>
		///     Determines the byte order marking offset (if any) from the
		///     given buffer.
		/// </summary>
		/// <param name="buffer">
		///     The buffer to examine.
		/// </param>
		/// <returns>
		///     The <see cref="int" /> representing the length of the byte order marking.
		/// </returns>
		private int GetBomOffset(byte[] buffer)
		{
			byte[] bom = encoding.GetPreamble();
			bool usesBom = true;
			for (int i = 0; i < bom.Length; ++i)
			{
				if (bom[i] != buffer[i])
				{
					usesBom = false;
				}
			}

			return usesBom ? bom.Length : 0;
		}

		/// <summary>
		///     Reads more data from the stream into the stream stack.
		/// </summary>
		/// <returns>
		///     The number of bytes read into the stream stack as an <see cref="int" />.
		/// </returns>
		private int StreamData()
		{
			var buffer = new byte[bufferSize];
			int amountRead = stream.Read(buffer, 0, bufferSize);

			PushToStack(buffer, amountRead);

			return amountRead;
		}

		/// <summary>
		///     Asynchronously reads more data from the stream into the stream stack.
		/// </summary>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The number of bytes read into the stream stack as an <see cref="int" />.
		/// </returns>
		private async Task<int> StreamDataAsync(CancellationToken cancellationToken = default)
		{
			var buffer = new byte[bufferSize];
			int amountRead = await stream.ReadAsync(buffer, 0, bufferSize, cancellationToken).ConfigureAwait(false);

			PushToStack(buffer, amountRead);

			return amountRead;
		}

		/// <summary>
		///     Push the data read from the stream into the stream stack.
		/// </summary>
		/// <param name="buffer">
		///     The data that was read from the stream.
		/// </param>
		/// <param name="amountRead">
		///     The number of bytes read from the stream.
		/// </param>
		private void PushToStack(byte[] buffer, int amountRead)
		{
			// We need to check if our stream is using our encodings
			// BOM, if it is we need to jump it.
			int bomOffset = GetBomOffset(buffer);

			// Sometimes we'll get a buffer that's smaller then we expect, chop it down
			// for the reader:
			if (amountRead - bomOffset > 0)
			{
				streamStack.Push(buffer, bomOffset, amountRead - bomOffset);
			}
		}

		#endregion
	}
}
