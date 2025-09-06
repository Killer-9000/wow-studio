#pragma once

#include "data/Misc.h"
#include "data/DBCFile.h"
#include <string>

class ClientData
{
public:
    static ClientData& Instance()
    {
        static ClientData instance;
        return instance;
    }

    // Location & locale
    std::string Location;
    ELocale     Locale;

    // Databases
    MapDBC MapDatabase;

private:
    ClientData()
        : MapDatabase(ELocale::Unk)
    {}
};

#define SClientData ClientData::Instance()