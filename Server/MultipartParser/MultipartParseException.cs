using System;

namespace MultipartFormData
{
	/// <summary>
	///     Represents a parsing problem occurring within the MultipartFormDataParser.
	/// </summary>
#if NET40
    [Serializable]
#endif
	public class MultipartParseException : Exception
	{
		#region Constructors and Destructors

		/// <summary>
		///     Initializes a new instance of the <see cref="MultipartParseException" /> class.
		/// </summary>
		/// <param name="message">
		///     The message.
		/// </param>
		public MultipartParseException(string message)
			: base(message)
		{
		}

		#endregion
	}
}
