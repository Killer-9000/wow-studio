#include "MappedFile.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#error "NOT INPLEMENTED"
#endif


MappedFile::MappedFile(std::string_view filename, MappedFileMode mode, MappedFileOpenMode openMode)
	: MappedFile(filename, 0, mode, openMode)
{
}

MappedFile::MappedFile(std::string_view filename, uint64_t fileSize, MappedFileMode mode, MappedFileOpenMode openMode)
{
#ifdef WIN32
	static const int DesiredAccess[] = { GENERIC_READ, GENERIC_WRITE, GENERIC_ALL };
	static const int FileShare[] = { FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE };
	static const int CreationDisposition[] = { CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS };
	static const int Protection[] = { PAGE_READONLY, PAGE_WRITECOPY, PAGE_READWRITE };
	static const int Access[] = { FILE_MAP_READ, FILE_MAP_WRITE, FILE_MAP_ALL_ACCESS };

	m_file = CreateFileA(filename.data(), DesiredAccess[(int)mode], FileShare[(int)mode], nullptr, CreationDisposition[(int)openMode], FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open file '%s' %i.\n", filename.data(), GetLastError());
		return;
	}

	m_fileMapping = CreateFileMappingA(m_file, nullptr, Protection[(int)mode], (fileSize >> 32 & UINT32_MAX), fileSize, nullptr);
	if (!m_fileMapping)
	{
		printf("Failed to open file mapping for file '%s' %i.\n", filename.data(), GetLastError());
		CloseHandle(m_file);
		return;
	}

	m_fileView = MapViewOfFile(m_fileMapping, Access[(int)mode], 0, 0, 0);
	if (!m_fileView)
	{
		printf("Failed to get map view for file '%s' %i.\n", filename.data(), GetLastError());
		CloseHandle(m_fileMapping);
		CloseHandle(m_file);
		return;
	}
#else
#error "NOT INPLEMENTED"
#endif

	m_open = true;
}

MappedFile::~MappedFile()
{
#ifdef WIN32
	UnmapViewOfFile(m_fileView);
	CloseHandle(m_fileMapping);
	CloseHandle(m_file);
#else
#error "NOT INPLEMENTED"
#endif
}

uint8_t* MappedFile::GetData()
{
#ifdef WIN32
	return (uint8_t*)m_fileView;
#else
#error "NOT INPLEMENTED"
#endif
}

const uint8_t* MappedFile::GetData() const
{
#ifdef WIN32
	return (uint8_t*)m_fileView;
#else
#error "NOT INPLEMENTED"
#endif
}

size_t MappedFile::GetSize()
{
#ifdef WIN32
	return GetFileSize(m_file, nullptr);
#else
#error "NOT INPLEMENTED"
#endif
}

const size_t MappedFile::GetSize() const
{
#ifdef WIN32
	return GetFileSize(m_file, nullptr);
#else
#error "NOT INPLEMENTED"
#endif
}

bool MappedFile::FlushChanges()
{
#ifdef WIN32
	return FlushChanges(0, 0);
#else
#error "NOT INPLEMENTED"
#endif
}

bool MappedFile::FlushChanges(uint64_t offset, uint64_t size)
{
#ifdef WIN32
	return FlushViewOfFile(&m_fileView + offset, size);
#else
#error "NOT INPLEMENTED"
#endif
}
