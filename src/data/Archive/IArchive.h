#pragma once

#include <string>

class IArchive
{
public:
	enum class ArchiveType
	{
		None,
		MPQ, // Incompatable with CASC
		CASC,
		Broken,
		Folder
	};

	virtual bool LoadArchive() = 0;
	virtual bool UnloadArchive() = 0;

	virtual bool ContainsFile(std::string_view) = 0;
	virtual bool OpenFile(std::string_view, uint8_t**, uint32_t*) = 0;

	ArchiveType GetArchiveType() const { return m_type; }
	const std::string& GetArchiveName() const { return m_filename; }

protected:
	IArchive(std::string name, ArchiveType type)
		: m_filename{ name }, m_type{ type }
	{

	}

	std::string m_filename;
	ArchiveType m_type;
};
