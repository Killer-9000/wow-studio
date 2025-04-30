#pragma once

#include <nlohmann/json.hpp>
#include <fstream>

struct Settings
{
	struct Project
	{
		std::string project_name;
		std::string project_path;
		std::string client_path;
		uint32_t client_version;
	};

	struct Graphics
	{
		bool vsync = true;
		float framelimit = 60.0f;
	};

	int curr_project = -1;
	std::vector<Project> projects;
	Graphics graphics;
};

void to_json(nlohmann::json& j, const Settings& s);
void from_json(const nlohmann::json& j, Settings& s);

void to_json(nlohmann::json& j, const Settings::Project& p);
void from_json(const nlohmann::json& j, Settings::Project& p);

void to_json(nlohmann::json& j, const Settings::Graphics& g);
void from_json(const nlohmann::json& j, Settings::Graphics& g);

class SettingsFile
{
	SettingsFile() { Load(); }
	~SettingsFile() { Save(); }
public:

	static SettingsFile& Instance()
	{
		static SettingsFile instance;
		return instance;
	}

	void Save();
	void Load();

	Settings _settings;
};

#define SSettingsFile SettingsFile::Instance()
