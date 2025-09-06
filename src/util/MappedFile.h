#pragma once

#include <cstring>
#include <string_view>
#include <stdint.h>

enum class MappedFileMode
{
	EREAD = 0,
	EWRITE,
	EBOTH
};

enum class MappedFileOpenMode
{
	ECREATE_NEW = 0,
	ECREATE_ALWAYS,
	EOPEN_EXISTING,
	EOPEN_ALWAYS,
};

class 
MappedFile
{
public:
	MappedFile() { }
	MappedFile(std::string_view filename, MappedFileMode mode, MappedFileOpenMode openMode);
	MappedFile(std::string_view filename, uint64_t fileSize, MappedFileMode mode, MappedFileOpenMode openMode);

	uint8_t* GetData();
	size_t GetSize();
	bool IsOpen() { return m_open; }

	const uint8_t* GetData() const;
	const size_t GetSize() const;
	bool IsOpen() const { return m_open; }
	
	void Close();

	bool FlushChanges();
	bool FlushChanges(uint64_t offset, uint64_t size);

	bool Resize(uint64_t newSize);

	template <typename T>
	T* Read(size_t count = 1, size_t offset = SIZE_MAX)
	{
		if (offset == SIZE_MAX)
		{
			offset = m_readPos;
			m_readPos += sizeof(T) * count;
		}
		else
			m_readPos = offset + sizeof(T) * count;

		return (T*)(GetData() + offset);
	}

	template <typename T>
	void Write(const T& data, size_t offset = SIZE_MAX)
	{
		Write(&data, 1, offset);
	}

	template <typename T>
	void Write(const T* data, size_t count = 1, size_t offset = SIZE_MAX)
	{
		if (offset == SIZE_MAX)
		{
			offset = m_readPos;
			m_readPos += sizeof(T) * count;
		}
		else
			m_readPos = offset + sizeof(T) * count;

		// If file is too small increase by 1KB, or if large enough then 1MB
		while (offset + sizeof(T) * count > GetSize())
			Resize(GetSize() + ((sizeof(T) * count > 1024) ? (1024 * 1024) : 1024));

		uint8_t* file = GetData();
		uint64_t test = *((uint64_t*)file);
		memcpy(file, data, sizeof(T) * count);
	}

private:
#if defined(PLATFORM_WIN32)
	void* m_file = nullptr;
	void* m_fileMapping = nullptr;
	uint8_t* m_fileView = nullptr;
	int   m_protection = -1;
	int   m_access = -1;
	std::string m_filename;
#elif defined(PLATFORM_LINUX)
	int m_file = -1;
	void* m_fileMapping = nullptr;
	size_t m_fileSize = 0;
#endif
	bool m_open = false;
	bool m_closed = false;
	size_t m_readPos = 0;
};
