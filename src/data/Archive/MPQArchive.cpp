#include "MPQArchive.h"
#include <array>

bool MPQArchive::LoadArchive()
{
	if (!SFileOpenArchive(m_filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY, &m_hArchive))
	{
		printf("Failed to open archive '%s', error: '%i'\n", m_filename.c_str(), GetLastError());
		return false;
	}

	printf("Opened archive '%s'\n", m_filename.c_str());
	return true;
}

bool MPQArchive::UnloadArchive()
{
	return SFileCloseArchive(m_hArchive);
}

const std::vector<std::string>& MPQArchive::GetArchiveLoadOrder()
{
	static const std::vector<std::string> s_335_archive_load_order =
	{
		"common{Number}.MPQ",
		"expansion.MPQ",
		"lichking.MPQ",
		"patch{Number}.MPQ",
		"patch{Letter}.MPQ",
		"{Locale}/locale-{Locale}.MPQ",
		"{Locale}/expansion-locale-{Locale}.MPQ",
		"{Locale}/expansion-speech-{Locale}.MPQ",
		"{Locale}/lichking-locale-{Locale}.MPQ",
		"{Locale}/lichking-speech-{Locale}.MPQ",
		"{Locale}/patch-{Locale}{Number}.MPQ",
		"{Locale}/patch-{Locale}{Letter}.MPQ"
	};

	return s_335_archive_load_order;
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
