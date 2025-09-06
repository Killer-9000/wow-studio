#include "Misc.h"

#if defined(PLATFORM_WIN32)
#elif defined(PLATFORM_LINUX)
#define stricmp strcasecmp
#endif

ELocale ELocale_FromString(char* locale)
{
	if (!locale || strlen(locale) != 4)
		return ELocale::Unk;

	if (!stricmp(locale, "enUS"))      return ELocale::enUS;
	else if (!stricmp(locale, "enGB")) return ELocale::enGB;
	else if (!stricmp(locale, "koKR")) return ELocale::koKR;
	else if (!stricmp(locale, "frFR")) return ELocale::frFR;
	else if (!stricmp(locale, "deDE")) return ELocale::deDE;
	else if (!stricmp(locale, "enCN")) return ELocale::enCN;
	else if (!stricmp(locale, "zhCN")) return ELocale::zhCN;
	else if (!stricmp(locale, "enTW")) return ELocale::enTW;
	else if (!stricmp(locale, "zhTW")) return ELocale::zhTW;
	else if (!stricmp(locale, "esES")) return ELocale::esES;
	else if (!stricmp(locale, "esMX")) return ELocale::esMX;
	else if (!stricmp(locale, "ruRU")) return ELocale::ruRU;
	else if (!stricmp(locale, "ptPT")) return ELocale::ptPT;
	else if (!stricmp(locale, "ptBR")) return ELocale::ptBR;
	else if (!stricmp(locale, "itIT")) return ELocale::itIT;
	return ELocale::Unk;
}

const char* ELocale_ToString(ELocale locale)
{
	switch (locale)
	{
	case ELocale::enUS: return "enUS";
	case ELocale::enGB: return "enGB";
	case ELocale::koKR: return "koKR";
	case ELocale::frFR: return "frFR";
	case ELocale::deDE: return "deDE";
	case ELocale::enCN: return "enCN";
	case ELocale::zhCN: return "zhCN";
	case ELocale::enTW: return "enTW";
	case ELocale::zhTW: return "zhTW";
	case ELocale::esES: return "esES";
	case ELocale::esMX: return "esMX";
	case ELocale::ruRU: return "ruRU";
	case ELocale::ptPT: return "ptPT";
	case ELocale::ptBR: return "ptBR";
	case ELocale::itIT: return "itIT";
	default: return "Unknown";
	}
}

glm::vec4 EExpansion_ToColour(EExpansion expansionID)
{
	switch (expansionID)
	{
	case EExpansion::TheBurningCrusade:  return glm::vec4(0.093f, 0.190f, 0.049f, 1.0f);
	case EExpansion::WrathOfTheLichKing:  return glm::vec4(0.046f, 0.136f, 0.196f, 1.0f);
	default: return glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
	}
}

const char* EExpansion_ToString(EExpansion expansionID)
{
	switch (expansionID)
	{
	case EExpansion::Vanilla: return "Vanilla";
	case EExpansion::TheBurningCrusade: return "The Burning Crusade";
	case EExpansion::WrathOfTheLichKing: return "Wrath Of The Lichking";
	case EExpansion::Cataclysm: return "Cataclysm";
	case EExpansion::MistsOfPandaria: return "Mists Of Pandaria";
	case EExpansion::WarlordsOfDraenor: return "Warlords Of Draenor";
	case EExpansion::Legion: return "Legion";
	case EExpansion::BattleForAzeroth: return "Battle For Azeroth";
	case EExpansion::Shadowlands: return "Shadowlands";
	case EExpansion::Dragonflight: return "Dragonflight";
	case EExpansion::TheWarWithin: return "The War Within";
	default: return "Unknown";
	}
}

EExpansion EExpansion_FromString(char* expansion)
{
	if (!expansion)
		return EExpansion::Unk;

	if (!stricmp(expansion, "Vanilla")) return EExpansion::Vanilla;
	else if (!stricmp(expansion, "The Burning Crusade")) return EExpansion::TheBurningCrusade;
	else if (!stricmp(expansion, "Wrath Of The Lichking")) return EExpansion::WrathOfTheLichKing;
	else if (!stricmp(expansion, "Cataclysm")) return EExpansion::Cataclysm;
	else if (!stricmp(expansion, "Mists Of Pandaria")) return EExpansion::MistsOfPandaria;
	else if (!stricmp(expansion, "Warlords Of Draenor")) return EExpansion::WarlordsOfDraenor;
	else if (!stricmp(expansion, "Legion")) return EExpansion::Legion;
	else if (!stricmp(expansion, "Battle For Azeroth")) return EExpansion::BattleForAzeroth;
	else if (!stricmp(expansion, "Shadowlands")) return EExpansion::Shadowlands;
	else if (!stricmp(expansion, "Dragonflight")) return EExpansion::Dragonflight;
	else if (!stricmp(expansion, "The War Within")) return EExpansion::TheWarWithin;
	return EExpansion::Unk;
}
