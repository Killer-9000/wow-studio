#pragma once

#include "data/Misc.h"
#include <filesystem>
#include <StormLib.h>
#include <string_view>

class ArchiveMgr
{
	// typedef std::vector<std::shared_ptr<IArchive>> ArchiveVector;

private:
	ArchiveMgr() { }
	~ArchiveMgr() { }

public:
	static ArchiveMgr* Instance()
	{
		static ArchiveMgr instance;
		return &instance;
	}

	// ArchiveVector::const_reference AddArchive(IArchive* archive)
	// {
	// 	auto& item = m_archives.emplace_back(archive);
	// 	item->LoadArchive();
	//
	// 	// Load listfile.
	// 	if (!item->LoadListfile(&m_filenames, &m_filenamesData, &m_filenamesSize, &m_filenamesPtr))
	// 		printf("Failed to load listfile for archive '%s'\n", archive->GetArchiveName().c_str());
	//
	// 	return item;
	// }
	//
	// void ClearArchives()
	// {
	// 	for (auto& archive : m_archives)
	// 		archive->UnloadArchive();
	// 	m_archives.clear();
	// }
	//
	// IArchive::ArchiveType GetArchiveType() const
	// {
	// 	IArchive::ArchiveType type = IArchive::ArchiveType::None;
	// 	for (const auto& archive : m_archives)
	// 		type = (archive->GetArchiveType() != IArchive::ArchiveType::Folder) ? archive->GetArchiveType() : type;
	// 	return type;
	// }
	//
	// const ArchiveVector& GetArchives() const
	// {
	// 	return m_archives;
	// }
	//
	// ArchiveVector::const_reference GetArchive(std::shared_ptr<IArchive> archive) const
	// {
	// 	return *std::ranges::find(m_archives, archive);
	// }
	//
	// ArchiveVector::const_reference GetArchive(IArchive* archive) const
	// {
	// 	auto ref = std::ranges::find_if(m_archives, [&](const std::shared_ptr<IArchive>& itr) {
	// 			return itr.get() == archive;
	// 		});
	//
	// 	return *ref;
	// }
	//
	// ArchiveVector::const_reference GetArchive(std::string_view name, IArchive::ArchiveType type) const
	// {
	// 	auto ref = std::ranges::find_if(m_archives, [&](const std::shared_ptr<IArchive>& itr) {
	// 			return itr->GetArchiveType() == type && itr->GetArchiveName() == name;
	// 		});
	//
	// 	return *ref;
	// }
	//
	// bool ContainsArchive(std::string_view name)
	// {
	// 	auto ref = std::ranges::find_if(m_archives, [&](const std::shared_ptr<IArchive>& itr) {
	// 		return itr->GetArchiveName() == name;
	// 		});
	//
	// 	return ref != m_archives.end();
	// }
	//
	// std::string_view GetFilename(ListfileMap::key_type filehash)
	// {
	// 	if (!m_filenames.contains(filehash))
	// 		return std::string_view();
	// 	return m_filenames[filehash];
	// }

	int LoadClientData(std::filesystem::path directory, EExpansion expansion, ELocale locale);
	bool OpenFile(std::string_view filename, uint8_t** outData, uint32_t* outSize);
	bool FileExists(std::string_view filename);

private:
	int LoadClientData_Vanilla(std::filesystem::path directory, ELocale locale);
	int LoadClientData_TBC(std::filesystem::path directory, ELocale locale);
	int LoadClientData_WotLK(std::filesystem::path directory, ELocale locale);
	int LoadClientData_Cata(std::filesystem::path directory, ELocale locale);
	int LoadClientData_MoP(std::filesystem::path directory, ELocale locale);
	
	int LoadClientData_Common(std::filesystem::path directory, ELocale locale, std::vector<const char*>& archives);

	HANDLE m_mpq = nullptr;

	// ArchiveVector m_archives;
	//
	// A list of filenames from the archives.
	// ListfileMap m_filenames;
	// const char* m_filenamesData;
	// size_t m_filenamesSize = 0;
	// const char* m_filenamesPtr;
};

#define SArchiveMgr ArchiveMgr::Instance()