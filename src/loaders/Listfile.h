#pragma once

#include <tsl/robin_map.h>
#include <filesystem>
#include <string>

class Listfile
{
	Listfile() { }
	~Listfile() { }
public:
	static Listfile* Instance()
	{
		static Listfile instance;
		return &instance;
	}

	bool LoadListfile(std::string_view filepath);

	void UnloadListfile()
	{
		m_idMap.clear();
		m_nameMap.clear();
		delete m_data;
		m_data = nullptr;
	}

private:
	char* m_data = nullptr;
	size_t m_dataSize = 0;
	tsl::robin_map<uint64_t, std::string_view> m_idMap;
	tsl::robin_map<std::string_view, uint64_t> m_nameMap;
};

#define S_LISTFILE Listfile::Instance()
