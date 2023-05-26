#pragma once

#include <string>

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

class MappedFile
{
public:
	MappedFile(std::string_view filename, MappedFileMode mode, MappedFileOpenMode openMode);
	MappedFile(std::string_view filename, uint64_t fileSize, MappedFileMode mode, MappedFileOpenMode openMode);
	~MappedFile();

	uint8_t* GetData();
	size_t GetSize();
	bool IsOpen() { return m_open; }

	const uint8_t* GetData() const;
	const size_t GetSize() const;
	bool IsOpen() const { return m_open; }
	
	bool FlushChanges();
	bool FlushChanges(uint64_t offset, uint64_t size);

private:
#ifdef WIN32
	void* m_file = nullptr;
	void* m_fileMapping = nullptr;
	void* m_fileView = nullptr;
#endif
	bool m_open = false;
};
