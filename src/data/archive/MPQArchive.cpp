#include "MPQArchive.h"
#include <array>
#include <immintrin.h>

__attribute__((target("sse4")))
void CleanListfile(uint8_t** listfileData, uint32_t* listfileSize)
{
	*listfileSize = (uint32_t)ceil((float)*listfileSize / sizeof(__m128i)) * sizeof(__m128i);
	*listfileData = (uint8_t*)realloc(*listfileData, *listfileSize);

	if (!*listfileData)
	{
		free(*listfileData);
		throw "Failed to reallocate listfile.";
		exit(-1);
	}

	static const __m128i mmSlashs = _mm_set1_epi8('\\');
	static const __m128i mmReturns = _mm_set1_epi8('\r');
	static const __m128i mmNewline = _mm_set1_epi8('\n');
	static const __m128i mmUpper = _mm_set1_epi8('Z');
	static const __m128i mmLower = _mm_set1_epi8('A');
	static const __m128i mmDiff = _mm_set1_epi8('a' - 'A');
	__m128i mmChars;
	__m128i mmMask;
	char* start = (char*)*listfileData;
	char* end = (char*)*listfileData + *listfileSize;
	for (; start < end; start += sizeof(__m128i))
	{
		// Load chars into 128bit value.
		mmChars = _mm_loadu_si128((__m128i_u*)start);

		// Check for uppercase
		// Set mask to 1
		// Anything less than lower should be 0
		// Anything more than upper should be 0
		mmMask = _mm_cmpeq_epi8(
			_mm_cmpgt_epi8(mmLower, mmChars),
			_mm_cmpgt_epi8(mmChars, mmUpper)
		);
		mmChars = _mm_blendv_epi8(mmChars, _mm_adds_epi8(mmChars, mmDiff), mmMask);

		// Check for backslashes.
		mmMask = _mm_cmpeq_epi8(mmChars, mmSlashs);
		mmChars = _mm_blendv_epi8(mmChars, _mm_set1_epi8('/'), mmMask);

		// Check for \r,\n and replace with null.
		mmMask = _mm_cmpeq_epi8(mmChars, mmReturns);
		mmChars = _mm_blendv_epi8(mmChars, _mm_set1_epi8('\0'), mmMask);
		mmMask = _mm_cmpeq_epi8(mmChars, mmNewline);
		mmChars = _mm_blendv_epi8(mmChars, _mm_set1_epi8('\0'), mmMask);

		// Load 128bit value into chars.
		_mm_storeu_si128((__m128i_u*)start, mmChars);
	}
}

bool MPQArchive::LoadArchive()
{
	if (!SFileOpenArchive(m_filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY, &m_hArchive))
	{
		printf("Failed to open archive '%s', error: '%i'\n", m_filename.c_str(), SErrGetLastError());
		return false;
	}

	printf("Opened archive '%s'\n", m_filename.c_str());
	return true;
}

bool MPQArchive::UnloadArchive()
{
	return SFileCloseArchive(m_hArchive);
}

const std::vector<std::string>& MPQArchive::GetArchiveLoadOrder(EExpansion expansion)
{
	static const std::vector<std::string> s_archive_load_order =
	{
		"alternate.MPQ",
		"art.MPQ",
		"speech.MPQ",
		"dbc.MPQ",
		"fonts.MPQ",
		"interface.MPQ",
		"misc.MPQ",
		"sound.MPQ",
		"wmo.MPQ",
		"terrain.MPQ",
		"texture.MPQ",
		"model.MPQ",

		"world.MPQ",
		"world2.MPQ",

		"common.MPQ",
		"common-2.MPQ",
		"expansion.MPQ",
		"expansion{Number}.MPQ",
		"lichking.MPQ",
		"patch.MPQ",
		"patch-{Number}.MPQ",
		"patch-{Letter}.MPQ",

		"wow-update-base-{Version}.MPQ",

		"{Locale}/locale-{Locale}.MPQ",
		"{Locale}/expansion-locale-{Locale}.MPQ",
		"{Locale}/expansion-speech-{Locale}.MPQ",
		"{Locale}/expansion{Number}-locale-{Locale}.MPQ",
		"{Locale}/expansion{Number}-speech-{Locale}.MPQ",
		"{Locale}/lichking-locale-{Locale}.MPQ",
		"{Locale}/lichking-speech-{Locale}.MPQ",
		"{Locale}/patch-{Locale}.MPQ",
		"{Locale}/patch-{Locale}-{Number}.MPQ",
		"{Locale}/patch-{Locale}-{Letter}.MPQ",

		"wow-update-{Locale}-{Version}.MPQ",

		"development.MPQ"
	};

	return s_archive_load_order;
}

bool MPQArchive::LoadListfile(ListfileMap* listfile, const char** listfileData, size_t* listfileSize, const char** listfilePtr)
{
	// TODO: Use the Listfile in loaders.
	uint8_t* data = nullptr; uint32_t size = 0;
	if (!OpenFile("(listfile)", &data, &size))
		return false;

	CleanListfile(&data, &size);

	size_t lineCount = 0;
	for (uint8_t* c = data; c < data + size; c++)
		if (*c == '\0') lineCount++;
	(*listfile).reserve(lineCount);

	uint8_t* forward = data;
	const uint8_t* current = data;
	uint8_t* end = data + size;
	while (forward < end)
	{
		if (*forward == '\0')
		{
			size_t hash = std::hash<const char*>()((const char*)current);
			if (listfile->contains(hash) == false)
				(*listfile)[hash] = std::string_view((const char*)current, forward - current);
			while (*forward == '\0' && forward < end) forward++;
			current = forward;
		}
		forward++;
	}

	return true;
}

bool MPQArchive::ContainsFile(std::string_view filename)
{
	return SFileHasFile(m_hArchive, filename.data());
}

bool MPQArchive::OpenFile(std::string_view filename, uint8_t** outData, uint32_t* outSize)
{
	HANDLE file = nullptr;
	if (!SFileOpenFileEx(m_hArchive, filename.data(), 0, &file))
		return false;

	*outSize = SFileGetFileSize(file, 0);
	if (*outData == nullptr)
		*outData = (uint8_t*)malloc(*outSize);
	if (!SFileReadFile(file, *outData, *outSize, 0, 0))
	{
		free(*outData);
		*outData = nullptr;
		*outSize = 0;
		SFileCloseFile(file);
		return false;
	}

	SFileCloseFile(file);
	return true;
}
