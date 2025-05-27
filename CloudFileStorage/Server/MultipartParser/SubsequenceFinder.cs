using System;

namespace MultipartFormData
{
	/// <summary>
	///     Provides methods to find a subsequence within a
	///     sequence.
	/// </summary>
	public class SubsequenceFinder
	{
		#region Public Methods and Operators

		/// <summary>
		///     Finds if a sequence exists within another sequence.
		/// </summary>
		/// <param name="haystack">
		///     The sequence to search.
		/// </param>
		/// <param name="needle">
		///     The sequence to look for.
		/// </param>
		/// <returns>
		///     The start position of the found sequence or -1 if nothing was found.
		/// </returns>
		public static int Search(byte[] haystack, byte[] needle)
		{
			return Search(haystack, needle, haystack.Length);
		}

		/// <summary>Finds if a sequence exists within another sequence.</summary>
		/// <param name="haystack">The sequence to search.</param>
		/// <param name="needle">The sequence to look for.</param>
		/// <param name="haystackLength">The length of the haystack to consider for searching.</param>
		/// <returns>The start position of the found sequence or -1 if nothing was found.</returns>
		/// <remarks>Inspired by https://stackoverflow.com/a/39021296/153084 .</remarks>
		public static int Search(byte[] haystack, byte[] needle, int haystackLength)
		{
			const int SEQUENCE_NOT_FOUND = -1;

			// Validate the parameters
			if (haystack == null || haystack.Length == 0) return SEQUENCE_NOT_FOUND;
			if (needle == null || needle.Length == 0) return SEQUENCE_NOT_FOUND;
			if (needle.Length > haystack.Length) return SEQUENCE_NOT_FOUND;
			if (haystackLength > haystack.Length || haystackLength < 1) throw new ArgumentException("Length must be between 1 and the length of the haystack.");

			int currentIndex = 0;
			int end = haystackLength - needle.Length; // past here no match is possible
			byte firstByte = needle[0]; // cached to tell compiler there's no aliasing

			while (currentIndex <= end)
			{
				// scan for first byte only. compiler-friendly.
				if (haystack[currentIndex] == firstByte)
				{
					// scan for rest of sequence
					for (int offset = 1; ; ++offset)
					{
						if (offset == needle.Length)
						{ // full sequence matched?
							return currentIndex;
						}
						else if (haystack[currentIndex + offset] != needle[offset])
						{
							break;
						}
					}
				}

				++currentIndex;
			}

			// end of array reached without match
			return SEQUENCE_NOT_FOUND;
		}

		#endregion
	}
}
