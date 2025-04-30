#include "SettingsFile.h"

void to_json(nlohmann::json& j, const Settings& s)
{
	j = nlohmann::json();

	j["curr_project"] = s.curr_project;
	j["projects"] = std::vector<Settings::Project>();

	for (size_t i = 0; i < s.projects.size(); i++)
	{
		j["projects"][i] = s.projects[i];
	}

	j["graphics"] = s.graphics;
}

void from_json(const nlohmann::json& j, Settings& s)
{
	s = Settings();

	if (j.contains("curr_project"))
		j.at("curr_project").get_to(s.curr_project);

	if (j.contains("projects"))
		j.at("projects").get_to(s.projects);

	if (j.contains("graphics"))
		j.at("graphics").get_to(s.graphics);
}

void to_json(nlohmann::json& j, const Settings::Project& p)
{
	j = nlohmann::json();

	j["project_name"] = p.project_name;
	j["project_path"] = p.project_path;
	j["client_path"] = p.client_path;
	j["client_version"] = p.client_version;
}

void from_json(const nlohmann::json& j, Settings::Project& p)
{
	p = Settings::Project();

	if (j.contains("project_name"))
		j.at("project_name").get_to(p.project_name);
	if (j.contains("project_path"))
		j.at("project_path").get_to(p.project_path);
	if (j.contains("client_path"))
		j.at("client_path").get_to(p.client_path);
	if (j.contains("client_version"))
		j.at("client_version").get_to(p.client_version);
}

void to_json(nlohmann::json& j, const Settings::Graphics& g)
{
	j = nlohmann::json();

	j["vsync"] = g.vsync;
	j["framelimit"] = g.framelimit;
}

void from_json(const nlohmann::json& j, Settings::Graphics& g)
{
	g = Settings::Graphics();

	if (j.contains("vsync"))
		j.at("vsync").get_to(g.vsync);
	if (j.contains("framelimit"))
		j.at("framelimit").get_to(g.framelimit);
}

void SettingsFile::Save()
{
	nlohmann::json json = _settings;

	auto file = std::ofstream("settings.json");
	file << json;
}

void SettingsFile::Load()
{
	nlohmann::json json;

	auto file = std::ifstream("settings.json");
	if (file.is_open())
		file >> json;

	_settings = json.get<Settings>();
}