#pragma once

#include "data/archive/IArchive.h"
#include "data/Misc.h"
#include <StormLib.h>
#include <vector>

class MPQArchive : public IArchive
{
public:
	MPQArchive(std::string filename)
		: IArchive(filename, ArchiveType::MPQ)
	{

	}

	virtual bool LoadArchive() override;
	virtual bool UnloadArchive() override;

	virtual bool ContainsFile(std::string_view) override;
	virtual bool OpenFile(std::string_view, uint8_t**, uint32_t*) override;

	virtual bool LoadListfile(ListfileMap* listfile, const char** listfileData, size_t* listfileSize, const char** listfilePtr) override;
	
	static const std::vector<std::string>& GetArchiveLoadOrder(EExpansion expansion);

private:
	HANDLE m_hArchive = nullptr;

};