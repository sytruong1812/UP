using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace MultipartFormData
{	
	public class MultipartFormBinaryDataParser : IMultipartFormBinaryDataParser
	{
		#region Constants and fields

		private readonly List<FilePart> _files;
		private readonly List<ParameterPartBinary> _parameters;

		#endregion

		#region Constructors and Destructors

		/// <summary>
		///     Initializes a new instance of the <see cref="MultipartFormBinaryDataParser"/> class.
		/// </summary>
		private MultipartFormBinaryDataParser()
		{
			_files = new List<FilePart>();
			_parameters = new List<ParameterPartBinary>();
		}

		#endregion

		#region Public Properties

		/// <summary>
		///     Gets the mapping of parameters parsed files. The name of a given field
		///     maps to the parsed file data.
		/// </summary>
		public IReadOnlyList<FilePart> Files => _files.AsReadOnly();

		/// <summary>
		///     Gets the parameters. Several ParameterParts may share the same name.
		/// </summary>
		public IReadOnlyList<ParameterPartBinary> Parameters => _parameters.AsReadOnly();

		#endregion

		#region Static Methods

		/// <summary>
		///     Parse the stream into a new instance of the <see cref="MultipartFormBinaryDataParser" /> class
		///     with the boundary, input encoding and buffer size.
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
		///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
		/// </param>
		/// <returns>
		///     A new instance of the <see cref="MultipartFormDataParser"/> class.
		/// </returns>
		public static MultipartFormBinaryDataParser Parse(Stream stream, Encoding encoding, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false)
		{
			return Parse(stream, null, encoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts);
		}

		/// <summary>
		///     Parse the stream into a new instance of the <see cref="MultipartFormBinaryDataParser" /> class
		///     with the boundary, input encoding and buffer size.
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
		///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
		/// </param>
		/// <returns>
		///     A new instance of the <see cref="MultipartFormDataParser"/> class.
		/// </returns>
		public static MultipartFormBinaryDataParser Parse(Stream stream, string boundary = null, Encoding encoding = null, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false)
		{
			var parser = new MultipartFormBinaryDataParser();
			parser.ParseStream(stream, boundary, encoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts);
			return parser;
		}

		/// <summary>
		///     Asynchronously parse the stream into a new instance of the <see cref="MultipartFormBinaryDataParser" /> class
		///     with the boundary, input encoding and buffer size.
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
		///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
		/// </param>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     A new instance of the <see cref="MultipartFormBinaryDataParser"/> class.
		/// </returns>
		public static Task<MultipartFormBinaryDataParser> ParseAsync(Stream stream, Encoding encoding, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false, CancellationToken cancellationToken = default)
		{
			return ParseAsync(stream, null, encoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts, cancellationToken);
		}

		/// <summary>
		///     Asynchronously parse the stream into a new instance of the <see cref="MultipartFormBinaryDataParser" /> class
		///     with the boundary, input encoding and buffer size.
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
		///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
		/// </param>
		/// <param name="cancellationToken">
		///     The cancellation token.
		/// </param>
		/// <returns>
		///     A new instance of the <see cref="MultipartFormBinaryDataParser"/> class.
		/// </returns>
		public static async Task<MultipartFormBinaryDataParser> ParseAsync(Stream stream, string boundary = null, Encoding encoding = null, int binaryBufferSize = Constants.DefaultBufferSize, string[] binaryMimeTypes = null, bool ignoreInvalidParts = false, CancellationToken cancellationToken = default)
		{
			var parser = new MultipartFormBinaryDataParser();
			await parser.ParseStreamAsync(stream, boundary, encoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts, cancellationToken).ConfigureAwait(false);
			return parser;
		}

        #endregion

        #region Private Methods

        /// <summary>
        ///     Parse the stream with the boundary, input encoding and buffer size.
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
        ///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
        /// </param>
        private void ParseStream(Stream stream, string boundary, Encoding encoding, int binaryBufferSize, string[] binaryMimeTypes, bool ignoreInvalidParts)
        {
            var streamingParser = new StreamingBinaryParser(stream, boundary, encoding ?? Constants.DefaultEncoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts);

            streamingParser.ParameterHandler += binaryParameterPart =>
            {
                _parameters.Add(binaryParameterPart);
            };

            streamingParser.FileHandler += (name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties) =>
            {
                if (partNumber == 0)
                {
                    // Create a new MemoryStream for the first part
                    var memoryStream = new MemoryStream();
                    _files.Add(new FilePart(name, fileName, memoryStream, additionalProperties, type, disposition));
                }

                // Write the buffer to the last file's MemoryStream
                Files[Files.Count - 1].Data.Write(buffer, 0, bytes);
            };

            streamingParser.Run();

            // Reset all the written memory streams so they can be read.
            foreach (var file in Files)
            {
                file.Data.Position = 0;
            }
        }

        /// <summary>
        ///     Parse the stream with the boundary, input encoding and buffer size.
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
        ///     By default the parser will throw an exception if it encounters an invalid part. Set this to true to ignore invalid parts.
        /// </param>
        /// <param name="cancellationToken">
        ///     The cancellation token.
        /// </param>
        private async Task ParseStreamAsync(Stream stream, string boundary, Encoding encoding, int binaryBufferSize, string[] binaryMimeTypes, bool ignoreInvalidParts, CancellationToken cancellationToken)
		{
			var desiredEncoding = encoding ?? Constants.DefaultEncoding;
			var streamingParser = new StreamingBinaryParser(stream, boundary, desiredEncoding, binaryBufferSize, binaryMimeTypes, ignoreInvalidParts);
			streamingParser.ParameterHandler += binaryParameterPart =>
			{
				_parameters.Add(binaryParameterPart);
			};

			streamingParser.FileHandler += (name, fileName, type, disposition, buffer, bytes, partNumber, additionalProperties) =>
			{
				if (partNumber == 0)
				{
                    // create file with first partNo
                    var memoryStream = new MemoryStream();
                    _files.Add(new FilePart(name, fileName, memoryStream, additionalProperties, type, disposition));
				}

				Files[Files.Count - 1].Data.Write(buffer, 0, bytes);
			};

			await streamingParser.RunAsync(cancellationToken).ConfigureAwait(false);

			// Reset all the written memory streams so they can be read.
			foreach (var file in Files)
			{
				file.Data.Position = 0;
			}
		}

		#endregion
	}
}
