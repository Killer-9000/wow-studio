#include "Listfile.h"
#include "util/MappedFile.h"
#include <immintrin.h>
#include <charconv>

__attribute__((target("sse4")))
bool Listfile::LoadListfile(std::string_view filepath)
{
	if (m_data)
		UnloadListfile();

	if (!std::filesystem::exists(filepath))
	{
		printf("WARN: Listfile '%s' doesn't exist.", filepath.data());
		return false;
	}

	// Load up data
	{
		MappedFile file(filepath, MappedFileMode::EREAD, MappedFileOpenMode::EOPEN_EXISTING);
		uint8_t* data = file.GetData();
		size_t fileSize = file.GetSize();

		size_t lineCount = 0;
		for (uint8_t* c = data; c < data + fileSize; c++)
			if (*c == '\n') lineCount++;

		fileSize = (size_t)ceil((float)fileSize / sizeof(__m128i)) * sizeof(__m128i);
		m_data = (char*)calloc(fileSize, 1);
		if (!m_data)
		{
			printf("ERR: Failed to allocate listfile memory!\n");
			return false;
		}
		memcpy(m_data, data, fileSize);
		m_dataSize = fileSize;

		m_idMap.reserve(lineCount);
		m_nameMap.reserve(lineCount);
	}

	// Tidy listfile
	{
		static const __m128i mmSlashs = _mm_set1_epi8('\\');
		static const __m128i mmSlash = _mm_set1_epi8('/');
		static const __m128i mmReturns = _mm_set1_epi8('\r');
		static const __m128i mmNewline = _mm_set1_epi8('\n');
		static const __m128i mmSemiColon = _mm_set1_epi8(';');
		static const __m128i mmNull = _mm_set1_epi8('\0');
		static const __m128i mmUpper = _mm_set1_epi8('Z');
		static const __m128i mmLower = _mm_set1_epi8('A');
		static const __m128i mmDiff = _mm_set1_epi8('a' - 'A');
		__m128i mmMask;

		for (char* mmData = m_data; mmData < m_data + m_dataSize; mmData += sizeof(__m128i))
		{
			// Need to load, as data is not aligned to 16.
			__m128i mmChars = _mm_loadu_si128((__m128i_u*)mmData);

			// Check for uppercase
			// Compare where upppercase, then using mask, add char difference.
			mmMask = _mm_cmpeq_epi8(_mm_cmpgt_epi8(mmLower, mmChars), _mm_cmpgt_epi8(mmChars, mmUpper));
			mmChars = _mm_blendv_epi8(mmChars, _mm_adds_epi8(mmChars, mmDiff), mmMask);

			// Check for backslashes.
			mmMask = _mm_cmpeq_epi8(mmChars, mmSlashs);
			mmChars = _mm_blendv_epi8(mmChars, mmSlash, mmMask);

			// Check for \r,\n and replace with null.
			mmMask = _mm_cmpeq_epi8(mmChars, mmReturns);
			mmChars = _mm_blendv_epi8(mmChars, mmNull, mmMask);
			mmMask = _mm_cmpeq_epi8(mmChars, mmNewline);
			mmChars = _mm_blendv_epi8(mmChars, mmNull, mmMask);

			mmMask = _mm_cmpeq_epi8(mmChars, mmSemiColon);
			mmChars = _mm_blendv_epi8(mmChars, mmNull, mmMask);

			_mm_storeu_si128((__m128i_u*)mmData, mmChars);
		}
	}

	// Read listfile
	{
		char* forward = m_data;

		// Skip pass any empty lines.
		while (*forward == '\0') forward++;

		uint64_t finalId = 0;
		const char* id = forward;
		const char* name = forward;
		bool afterId = false;
		for (; forward < m_data + m_dataSize; forward++)
		{
			// End of a line or id.
			if (*forward != '\0')
				continue;

			// Name
			if (afterId)
			{
				std::from_chars(id, name - 1, finalId);
				m_nameMap[name] = finalId;
				m_idMap[finalId] = name;

				while (*forward == '\0' && forward < m_data + m_dataSize) forward++;

				id = forward;
				afterId = false;
			}
			// ID
			else
			{
				name = forward + 1;
				afterId = true;
			}
		}
	}

	return true;
}
