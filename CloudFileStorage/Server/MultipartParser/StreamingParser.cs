using System;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace MultipartFormData
{
	public class StreamingParser : IStreamingParser
	{
		/// <summary>
		///     List of mimetypes that should be detected as file.
		/// </summary>
		private readonly string[] binaryMimeTypes;

		/// <summary>
		///     The stream we are parsing.
		/// </summary>
		private readonly Stream stream;

		/// <summary>
		///     Determines if we should throw an exception when we enconter an invalid part or ignore it.
		/// </summary>
		private readonly bool ignoreInvalidParts;

		/// <summary>
		///     The boundary of the multipart message  as a string.
		/// </summary>
		private readonly string boundary;

		/// <summary>
		///     Initializes a new instance of the <see cref="StreamingParser" /> class
		///     with the boundary, stream, input encoding and buffer size.
		/// </summary>
		/// <param name="stream">
		///     The stream containing the multipart data.
		/// </param>
		/// <param name="encoding">
		///     The encoding of the multipart data.
		/// </param>
		/// <param name="binaryBufferSize">
		///     The size of the buffer to use for parsing the multipart form data. This must be larger
		///     then (size of boundary + 4 + # bytes in newline).
		/// </param>
		/// <param name="binaryMimeTypes">
		///     List of mimetypes that should be detected as file.
		/// </param>
		/// <param name="ignoreInvalidParts">
		///     By default the parser will throw an exception if it encounters an invalid part. set this to true to ignore invalid parts.
		/// </param>
		public StreamingParser(Stream stream, Encoding encoding, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false)
			: this(stream, null, encoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts)
		{
		}

		/// <summary>
		///     Initializes a new instance of the <see cref="StreamingParser" /> class
		///     with the boundary, stream, input encoding and buffer size.
		/// </summary>
		/// <param name="stream">
		///     The stream containing the multipart data.
		/// </param>
		/// <param name="boundary">
		///     The multipart/form-data boundary. This should be the value
		///     returned by the request header.
		/// </param>
		/// <param name="encoding">
		///     The encoding of the multipart data.
		/// </param>
		/// <param name="binaryBufferSize">
		///     The size of the buffer to use for parsing the multipart form data. This must be larger
		///     then (size of boundary + 4 + # bytes in newline).
		/// </param>
		/// <param name="binaryMimeTypes">
		///     List of mimetypes that should be detected as file.
		/// </param>
		/// <param name="ignoreInvalidParts">
		///     By default the parser will throw an exception if it encounters an invalid part. set this to true to ignore invalid parts.
		/// </param>
		public StreamingParser(Stream stream, string boundary = null, Encoding encoding = null, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false)
		{
			if (stream == null || stream == Stream.Null) { throw new ArgumentNullException(nameof(stream)); }

			this.stream = stream;
			this.boundary = boundary;
			Encoding = encoding ?? Constants.DefaultEncoding;
			BinaryBufferSize = binaryBufferSize;
			this.binaryMimeTypes = binaryMimeTypes ?? Constants.DefaultBinaryMimeTypes;
			this.ignoreInvalidParts = ignoreInvalidParts;
		}

		/// <summary>
		///     Begins executing the parser. This should be called after all handlers have been set.
		/// </summary>
		public void Run()
		{
			var streamingParser = new StreamingBinaryParser(stream, boundary, Encoding, BinaryBufferSize, binaryMimeTypes, ignoreInvalidParts);
			streamingParser.ParameterHandler += binaryParameterPart =>
			{
				ParameterHandler?.Invoke(new ParameterPart(binaryParameterPart.Name, binaryParameterPart.ToString(Encoding)));
			};

			streamingParser.FileHandler += (name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties) =>
			{
				FileHandler?.Invoke(name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties);
			};

			streamingParser.Run();
		}

		/// <summary>
		///     Begins executing the parser asynchronously. This should be called after all handlers have been set.
		/// </summary>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     The asynchronous task.
		/// </returns>
		public async Task RunAsync(CancellationToken cancellationToken = default)
		{
			var streamingParser = new StreamingBinaryParser(stream, boundary, Encoding ?? Constants.DefaultEncoding, BinaryBufferSize, binaryMimeTypes, ignoreInvalidParts);
			streamingParser.ParameterHandler += binaryParameterPart =>
			{
				ParameterHandler?.Invoke(new ParameterPart(binaryParameterPart.Name, binaryParameterPart.ToString(Encoding)));
			};

			streamingParser.FileHandler += (name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties) =>
			{
				FileHandler?.Invoke(name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties);
			};

			await streamingParser.RunAsync().ConfigureAwait(false);
		}

		/// <summary>
		/// Gets the binary buffer size.
		/// </summary>
		public int BinaryBufferSize { get; private set; }

		/// <summary>
		/// Gets the encoding.
		/// </summary>
		public Encoding Encoding { get; private set; }

		/// <summary>
		/// Gets or sets the FileHandler. Delegates attached to this property will receive sequential file stream data from this parser.
		/// </summary>
		public FileStreamDelegate FileHandler { get; set; }

		/// <summary>
		/// Gets or sets the ParameterHandler. Delegates attached to this property will receive parameter data.
		/// </summary>
		public ParameterDelegate ParameterHandler { get; set; }

		/// <summary>
		/// Gets or sets the StreamClosedHandler. Delegates attached to this property will be notified when the source stream is exhausted.
		/// </summary>
		public StreamClosedDelegate StreamClosedHandler { get; set; }
	}
}
