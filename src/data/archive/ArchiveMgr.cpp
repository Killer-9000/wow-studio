#include "ArchiveMgr.h"
#include <fmt/printf.h>
#include <regex>

int ArchiveMgr::LoadClientData(std::filesystem::path directory, EExpansion expansion, ELocale locale)
{
    switch (expansion)
    {
    case EExpansion::Vanilla: return LoadClientData_Vanilla(directory, locale);
    case EExpansion::TheBurningCrusade: return LoadClientData_TBC(directory, locale);
    case EExpansion::WrathOfTheLichKing: return LoadClientData_WotLK(directory, locale);
    case EExpansion::Cataclysm: return LoadClientData_Cata(directory, locale);
    case EExpansion::MistsOfPandaria: return LoadClientData_MoP(directory, locale);
    
    default: assert(false && "Unknown expansion");
    }
}

bool ArchiveMgr::OpenFile(std::string_view filename, uint8_t** outData, uint32_t* outSize)
{
    if (!outData || !outSize || filename == "")
        return false;

    // TODO: Implement

    return false;
}

bool ArchiveMgr::FileExists(std::string_view filename)
{
    if (filename == "")
        return false;
        
    // TODO: Implement

    return false;
}

int ArchiveMgr::LoadClientData_Vanilla(std::filesystem::path directory, ELocale locale)
{
    return 1;
}

int ArchiveMgr::LoadClientData_TBC(std::filesystem::path directory, ELocale locale)
{
    return 1;
}

int ArchiveMgr::LoadClientData_WotLK(std::filesystem::path directory, ELocale locale)
{
    static std::vector<const char*> s_archives = {
        "common.MPQ",
        "common-2.MPQ",
        "expansion.MPQ",
        "lichking.MPQ",
        "patch.MPQ",
        "patch-{N}.MPQ",
        "patch-{C}.MPQ",
        "{L}/locale-{L}.MPQ",
        "{L}/speech-{L}.MPQ",
        "{L}/expansion-locale-{L}.MPQ",
        "{L}/expansion-speech-{L}.MPQ",
        "{L}/lichking-locale-{L}.MPQ",
        "{L}/lichking-speech-{L}.MPQ",
        "{L}/patch-{L}.MPQ",
        "{L}/patch-{L}-{N}.MPQ",
        "{L}/patch-{L}-{C}.MPQ"
    };

    return LoadClientData_Common(directory, locale, s_archives);
}

int ArchiveMgr::LoadClientData_Cata(std::filesystem::path directory, ELocale locale)
{
    return 1;
}

int ArchiveMgr::LoadClientData_MoP(std::filesystem::path directory, ELocale locale)
{
    static std::vector<const char*> s_archives = {
        "alternate.MPQ",
        "art.MPQ",
        "expansion1.MPQ",
        "expansion2.MPQ",
        "expansion3.MPQ",
        "expansion4.MPQ",
        "interface.MPQ",
        "itemtexture.MPQ",
        "misc.MPQ",
        "model.MPQ",
        "sound.MPQ",
        "texture.MPQ",
        "world.MPQ",
        "world2.MPQ",
        "wow-update-base-{V}.MPQ",
        "{L}/locale-{L}.MPQ",
        "{L}/speech-{L}.MPQ",
        "{L}/expansion1-speech-{L}.MPQ",
        "{L}/expansion2-speech-{L}.MPQ",
        "{L}/expansion3-speech-{L}.MPQ",
        "{L}/expansion4-speech-{L}.MPQ",
        "{L}/wow-update-{L}-{V}.MPQ",
    };

    return LoadClientData_Common(directory, locale, s_archives);
}

int ArchiveMgr::LoadClientData_Common(std::filesystem::path directory, ELocale locale, std::vector<const char*>& archives)
{
    std::string localeString = ELocale_ToString(locale);
    std::string archiveName;

    archiveName = directory / "Data" / archives[0];
    if (!SFileOpenArchive(archiveName.c_str(), 0, MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_READ_ONLY, &m_mpq))
    {
        return 1;
    }

    for (int i = 1; i < archives.size(); i++)
    {
        archiveName = directory / "Data" / archives[i];
        size_t pos;

        while ((pos = archiveName.find("{L}")) != std::string::npos)
            archiveName.replace(pos, 3, localeString);

        if (pos = archiveName.find("{N}"); pos != std::string::npos)
        {
            std::string archiveName2;
            for (int i = 2; i < 10; i++)
            {
                archiveName2 = archiveName;
                archiveName2.replace(pos, 3, fmt::sprintf("%u", i));
                SFileOpenPatchArchive(m_mpq, archiveName2.c_str(), "", MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_READ_ONLY);
            }
        }
        else if (pos = archiveName.find("{C}"); pos != std::string::npos)
        {
            std::string archiveName2;
            for (int i = 'A'; i <= 'Z'; i++)
            {
                archiveName2 = archiveName;
                archiveName2.replace(pos, 3, fmt::sprintf("%c", i));
                SFileOpenPatchArchive(m_mpq, archiveName2.c_str(), "", MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_READ_ONLY);
            }
        }
        else if (pos = archiveName.find("{V}"); pos != std::string::npos)
        {
            // std::string archiveName2;
            archiveName.replace(pos, 3, "[0-9]{1,5}");
            auto dir = std::filesystem::path(archiveName).parent_path();
            auto pattern = std::regex(std::filesystem::path(archiveName).filename().string());
            for (auto& file : std::filesystem::directory_iterator(dir))
            {
                if (!file.is_regular_file())
                    continue;

                if (std::regex_match(file.path().filename().string(), pattern))
                {
                    SFileOpenPatchArchive(m_mpq, file.path().c_str(), "", MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_READ_ONLY);
                }
            }
        }
        else
        {
            if (!SFileOpenPatchArchive(m_mpq, archiveName.c_str(), "", MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_READ_ONLY))
            {
                SFileCloseArchive(m_mpq);
                m_mpq = nullptr;
                return 1;
            }
        }

    }

    return 0;
}