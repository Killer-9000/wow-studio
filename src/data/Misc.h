#pragma once

#include <glm/glm.hpp>
#include <string.h>

enum class ELocale
{
	Unk = -1,
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
	Unk2,
	MAX
};

const std::array<const char*, 15> s_localeNames =
{
	"enUS",
	"enGB",
	"koKR",
	"frFR",
	"deDE",
	"enCN",
	"zhCN",
	"enTW",
	"zhTW",
	"esES",
	"esMX",
	"ruRU",
	"ptPT",
	"ptBR",
	"itIT"
};

enum class EExpansion
{
	Unk = -1,
	Vanilla = 0,
	TheBurningCrusade,
	WrathOfTheLichKing,
	Cataclysm,
	MistsOfPandaria,
	WarlordsOfDraenor,
	Legion,
	BattleForAzeroth,
	Shadowlands,
	Dragonflight,
	TheWarWithin,
	MAX
};

const std::array<const char*, 11> s_expansionNames =
{
	"Vanilla",
	"The burning crusade",
	"Wrath of the lich king",
	"Cataclysm",
	"Mists of pandaria",
	"Warlords of draenor",
	"Legion",
	"Battle for azeroth",
	"Shadowlands",
	"Dragonflight",
	"The war within"
};

ELocale ELocale_FromString(char* locale);
const char* ELocale_ToString(ELocale locale);
glm::vec4 EExpansion_ToColour(EExpansion expansionID);
const char* EExpansion_ToString(EExpansion expansionID);
EExpansion EExpansion_FromString(char* expansion);