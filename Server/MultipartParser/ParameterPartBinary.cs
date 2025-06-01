using System;
using System.Linq;
using System.Text;
using System.Collections.Generic;

namespace MultipartFormData
{
	/// <summary>
	///     Represents the binary data of a single parameter extracted from a multipart/form-data stream.
	/// </summary>
	/// <remarks>
	///     For our purposes a "parameter" is defined as any non-file data
	///     in the multipart/form-data stream.
	/// </remarks>
	public class ParameterPartBinary
	{
		/// <summary>
		///     Initializes a new instance of the <see cref="ParameterPartBinary" /> class.
		/// </summary>
		/// <param name="name">
		///     The name.
		/// </param>
		/// <param name="data">
		///     The data.
		/// </param>
		public ParameterPartBinary(string name, IEnumerable<byte[]> data)
		{
			Name = name;
			Data = data;
		}

		/// <summary>
		///     Gets the data.
		/// </summary>
		public IEnumerable<byte[]> Data { get; }

		/// <summary>
		///     Gets the name.
		/// </summary>
		public string Name { get; }

		/// <summary>
		/// Get the binary data expressed as a string.
		/// </summary>
		/// <returns>The binary data expressed as a string.</returns>
		public override string ToString()
		{
			return ToString(Constants.DefaultEncoding);
		}

		/// <summary>
		/// Get the binary data expressed as a string.
		/// </summary>
		/// <param name="encoding">The encoding used to convert the binary data into a string.</param>
		/// <returns>The binary data expressed as a string.</returns>
		public string ToString(Encoding encoding)
		{
			if (encoding == null) { throw new ArgumentNullException(nameof(encoding)); }

			return string.Join(Environment.NewLine, Data.Select(line => encoding.GetString(line)));
		}
	}
}
