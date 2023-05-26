#pragma once

#include "data/Archive/ArchiveMgr.h"

#include <string_view>
#include <stdint.h>

enum WowLocaleEnum
{
	enUS = 0,
	enGB,
	koKR,
	frFR,
	deDE,
	enCN,
	zhCN,
	enTW,
	zhTW,
	esES,
	esMX,
	ruRU,
	ptPT,
	ptBR,
	itIT,
	Unk,
	Mask,
	MAX
};

class DBCFile
{
public:
	DBCFile(WowLocaleEnum locale) : m_locale(locale) { }
	~DBCFile() { free(m_buffer); }

	bool LoadFile(const char* filename)
	{
		// Just incase LoadFile is called multiple times.
		free(m_buffer);
		m_buffer = nullptr;
		m_data = nullptr;
		m_strings = nullptr;
		m_header = nullptr;

		// Open file.
		uint32_t fileSize = 0;
		if (!SArchiveMgr->OpenFile(filename, &m_buffer, &fileSize))
			return false;

		// Check contains at least the header.
		if (fileSize < sizeof(Header))
			return false;

		// Set the header ptr.
		m_header = (Header*)m_buffer;
		
		// Check contains record data.
		if (fileSize < sizeof(Header) + m_header->recordSize * m_header->recordCount)
			return false;

		// Set the data ptr.
		m_data = m_buffer + sizeof(Header);

		// Check contains the string data.
		if (fileSize < sizeof(Header) + m_header->recordSize * m_header->recordCount + m_header->stringBlockSize)
			return false;

		// Set the strings data.
		m_strings = (char*)m_data + m_header->recordSize * m_header->recordCount;
		
		return true;
	}

	uint32_t GetMaxID()
	{
		if (m_maxID != 0)
			return m_maxID;

		uint32_t* id = (uint32_t*)m_data;
		while (id < (uint32_t*)m_strings)
		{
			if (*id > m_maxID)
				m_maxID = *id;
			id += m_header->recordSize;
		}

		return m_maxID;
	}

	uint32_t GetMinID()
	{
		if (m_minID != UINT32_MAX)
			return m_minID;

		uint32_t* id = (uint32_t*)m_data;
		while (id < (uint32_t*)m_strings)
		{
			if (*id < m_minID)
				m_minID = *id;
			id += m_header->recordSize;
		}

		return m_minID;
	}

protected:
	// Everything else comes from the main buffer, that way there isn't multiple sets of data.
	uint8_t* m_buffer = nullptr;
	uint8_t* m_data = nullptr;
	char* m_strings = nullptr;

	uint32_t m_maxID = 0;
	uint32_t m_minID = UINT32_MAX;

	struct Header
	{
		uint32_t magic;
		uint32_t recordCount;
		uint32_t fieldCount;
		uint32_t recordSize;
		uint32_t stringBlockSize;
	} *m_header = nullptr;

	WowLocaleEnum m_locale;
};

class MapDBC : public DBCFile
{
public:
	MapDBC(WowLocaleEnum locale) : DBCFile(locale) { }
	~MapDBC() {  }

	bool LoadFile()
	{
		bool err = DBCFile::LoadFile("DBFilesClient\\Map.dbc");
		if (err)
			m_records.resize(m_header->recordCount);
		return err;
	}

	struct Record
	{
		Record() { }

		Record(MapDBC* dbc, uint32_t* data, WowLocaleEnum locale)
		{
			ID = *data++;
			Directory = std::string_view(dbc->m_strings + *data++);
			InstanceType = *data++;
			Flags = *data++;
			PVP = *data++;
			MapName = std::string_view(dbc->m_strings + *(data + locale)); data += WowLocaleEnum::MAX;
			AreaTableID = *data++;
			MapDescription0 = std::string_view(dbc->m_strings + *(data + locale)); data += WowLocaleEnum::MAX;
			MapDescription1 = std::string_view(dbc->m_strings + *(data + locale)); data += WowLocaleEnum::MAX;
			LoadingScreenID = *data++;
			MinimapIconScale = *((float*)data++);
			CorpseMapID = *data++;
			Corpse[0] = *data++;
			Corpse[1] = *data++;
			TimeOfDayOverride = *data++;
			ExpansionID = *data++;
			RaidOffset = *data++;
			MaxPlayers = *data++;
		}

		Record(MapDBC* dbc, size_t index, WowLocaleEnum locale)
		{
			uint8_t* data = dbc->m_data + index * dbc->m_header->recordSize;
			if (data >= (uint8_t*)dbc->m_strings)
				return;

			*this = Record(dbc, (uint32_t*)data, locale);
		}

		uint32_t ID = UINT32_MAX;
		std::string_view Directory;
		uint32_t InstanceType;
		uint32_t Flags;
		uint32_t PVP;
		std::string_view MapName;
		uint32_t AreaTableID;
		std::string_view MapDescription0;
		std::string_view MapDescription1;
		uint32_t LoadingScreenID;
		float MinimapIconScale;
		int32_t CorpseMapID;
		uint32_t Corpse[2];
		int32_t TimeOfDayOverride;
		uint32_t ExpansionID;
		uint32_t RaidOffset;
		uint32_t MaxPlayers;
	};

	Record* GetRecord(uint32_t recordID)
	{
		for (int i = 0; i < m_records.size(); i++)
			if (m_records[i] && m_records[i]->ID == recordID)
				return m_records[i].get();

		size_t index = 0;
		uint32_t* data = (uint32_t*)m_data;
		while (*data != recordID && data < (uint32_t*)m_strings)
		{
			data += m_header->recordSize;
			index++;
		}

		if (data > (uint32_t*)m_strings)
			return nullptr;

		m_records[index] = std::make_shared<Record>(this, index, m_locale);

		return m_records[index].get();
	}

	Record* GetRecordAlt(size_t index)
	{
		if (m_records[index].get() == nullptr)
			m_records[index] = std::make_shared<Record>(this, index, m_locale);
		if (m_records[index]->ID == UINT32_MAX)
			return nullptr;
		return m_records[index].get();
	}

	size_t GetRecordCount()
	{
		return m_records.size();
	}

	std::vector<std::shared_ptr<Record>> m_records;
};