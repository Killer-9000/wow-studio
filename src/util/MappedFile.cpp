#include "MappedFile.h"

#include <filesystem>

#if defined(PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(PLATFORM_LINUX)
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#else
#error "NOT INPLEMENTED"
#endif

MappedFile::MappedFile(std::string_view filename, MappedFileMode mode, MappedFileOpenMode openMode)
	: MappedFile(filename, 0, mode, openMode)
{
}

MappedFile::MappedFile(std::string_view filename, uint64_t fileSize, MappedFileMode mode, MappedFileOpenMode openMode)
{
#if defined(PLATFORM_WIN32)
	static const int DesiredAccess[] = { GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE };
	static const int FileShare[] = { FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE };
	static const int CreationDisposition[] = { CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS };
	static const int Protection[] = { PAGE_READONLY, PAGE_READWRITE, PAGE_READWRITE };
	static const int Access[] = { FILE_MAP_READ, FILE_MAP_WRITE, FILE_MAP_READ | FILE_MAP_WRITE };

	m_filename = filename;
	m_file = CreateFileA(filename.data(), DesiredAccess[(int)mode], FileShare[(int)mode], nullptr, CreationDisposition[(int)openMode], FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open file '%s' %i.\n", filename.data(), GetLastError());
		return;
	}

	m_protection = Protection[(int)mode] | SEC_RESERVE;
	m_fileMapping = CreateFileMappingA(m_file, nullptr, m_protection, (fileSize >> 32 & UINT32_MAX), fileSize, nullptr);
	if (!m_fileMapping)
	{
		printf("Failed to open file mapping for file '%s' %i.\n", filename.data(), GetLastError());
		CloseHandle(m_file);
		return;
	}

	m_access = Access[(int)mode];
	m_fileView = (uint8_t*)MapViewOfFile(m_fileMapping, m_access, 0, 0, fileSize);
	if (!m_fileView)
	{
		printf("Failed to get map view for file '%s' %i.\n", filename.data(), GetLastError());
		CloseHandle(m_fileMapping);
		CloseHandle(m_file);
		return;
	}
#elif defined(PLATFORM_LINUX)
	int flags = 0;

	flags |= (mode == MappedFileMode::EREAD  ? O_RDONLY : 0);
	flags |= (mode == MappedFileMode::EWRITE ? O_RDWR   : 0);
	flags |= (mode == MappedFileMode::EBOTH  ? O_RDWR   : 0);

	flags |= (openMode == MappedFileOpenMode::ECREATE_NEW    ? O_CREAT | O_EXCL : 0);
	flags |= (openMode == MappedFileOpenMode::ECREATE_ALWAYS ? O_CREAT : 0);
	flags |= (openMode == MappedFileOpenMode::EOPEN_EXISTING ? 0 : 0);
	flags |= (openMode == MappedFileOpenMode::EOPEN_ALWAYS   ? O_CREAT : 0);

	m_file = open64(filename.data(), flags);
	if (m_file < 0)
	{
		printf("Failed to open file '%s' %s.\n", filename.data(), strerror(-m_file));
		return;
	}

	if (fileSize == 0)
	{
		fileSize = lseek64(m_file, 0, SEEK_END);
		lseek64(m_file, 0, SEEK_SET);
	}
	else
		fcntl64(m_file, F_SETPIPE_SZ, fileSize);

	flags = 0;
	flags |= (mode == MappedFileMode::EREAD  ? PROT_READ : 0);
	flags |= (mode == MappedFileMode::EWRITE ? PROT_READ | PROT_WRITE : 0);
	flags |= (mode == MappedFileMode::EBOTH  ? PROT_READ | PROT_WRITE : 0);

	m_fileMapping = mmap64(NULL, fileSize, flags, MAP_PRIVATE, m_file, 0);
	if (m_fileMapping == MAP_FAILED)
	{
		printf("Failed to map file '%s' %s.\n", filename.data(), strerror(errno));
		close(m_file);
		std::filesystem::remove(filename);
		return;
	}

	m_fileSize = fileSize;

#else
#error "NOT INPLEMENTED"
#endif

	m_open = true;
}

uint8_t* MappedFile::GetData()
{
#if defined(PLATFORM_WIN32)
	return (uint8_t*)m_fileView;
#elif defined(PLATFORM_LINUX)
	return (uint8_t*)m_fileMapping;
#else
#error "NOT INPLEMENTED"
#endif
}

const uint8_t* MappedFile::GetData() const
{
#if defined(PLATFORM_WIN32)
	return (uint8_t*)m_fileView;
#elif defined(PLATFORM_LINUX)
	return (uint8_t*)m_fileMapping;
#else
#error "NOT INPLEMENTED"
#endif
}

size_t MappedFile::GetSize()
{
#if defined(PLATFORM_WIN32)
	return GetFileSize(m_file, nullptr);
#elif defined(PLATFORM_LINUX)
	return m_fileSize;
#else
#error "NOT INPLEMENTED"
#endif
}

const size_t MappedFile::GetSize() const
{
#if defined(PLATFORM_WIN32)
	return GetFileSize(m_file, nullptr);
#elif defined(PLATFORM_LINUX)
	return m_fileSize;
#else
#error "NOT INPLEMENTED"
#endif
}

void MappedFile::Close()
{
#if defined(PLATFORM_WIN32)
	UnmapViewOfFile(m_fileView); m_fileView = nullptr;
	CloseHandle(m_fileMapping); m_fileMapping = nullptr;
	CloseHandle(m_file); m_file = nullptr;
#elif defined(PLATFORM_LINUX)
	munmap(m_fileMapping, m_fileSize);
	close(m_file);
#else
#error "NOT INPLEMENTED"
#endif
}

bool MappedFile::FlushChanges()
{
#if defined(PLATFORM_WIN32)
	return FlushChanges(0, 0);
#elif defined(PLATFORM_LINUX)
	return msync(m_fileMapping, m_fileSize, MS_SYNC);
#else
#error "NOT INPLEMENTED"
#endif
}

bool MappedFile::FlushChanges(uint64_t offset, uint64_t size)
{
#if defined(PLATFORM_WIN32)
	return FlushViewOfFile((uint8_t*)m_fileView + offset, size);
#elif defined(PLATFORM_LINUX)
	return msync((uint8_t*)m_fileMapping + offset, size, MS_SYNC);
#else
#error "NOT INPLEMENTED"
#endif
}

bool MappedFile::Resize(uint64_t newSize)
{
#if defined(PLATFORM_WIN32)
	UnmapViewOfFile(m_fileView);
	CloseHandle(m_fileMapping);
	m_fileMapping = CreateFileMappingA(m_file, nullptr, m_protection, (newSize >> 32 & UINT32_MAX), newSize, nullptr);
	if (!m_fileMapping)
	{
		printf("Failed to open file mapping for file '%s' %i.\n", m_filename.data(), GetLastError());
		CloseHandle(m_file);
		return false;
	}
	m_fileView = (uint8_t*)MapViewOfFile(m_fileMapping, m_access, 0, 0, newSize);
	if (!m_fileView)
	{
		printf("Failed to get map view for file '%s' %i.\n", m_filename.data(), GetLastError());
		CloseHandle(m_fileMapping);
		CloseHandle(m_file);
		return false;
	}
	return true;
#elif defined(PLATFORM_LINUX)
	m_fileMapping = mremap(m_fileMapping, m_fileSize, newSize, MREMAP_MAYMOVE);
	m_fileSize = newSize;
	return m_fileMapping != nullptr;
#else
#error "NOT INPLEMENTED"
#endif
}