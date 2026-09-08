#include "Config.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>

void Config::load(std::string path)
{
	YAML::Node config = YAML::LoadFile(path);

	window.width = config["Window.width"].as<int>();
	window.height = config["Window.height"].as<int>();
	window.title = config["Window.title"].as<std::string>();
	window.fullscreen = config["Window.fullscreen"].as<bool>();


	ui.isCreditsPanelOpen  = config["UI.isCreditsPanelOpen"].as<bool>();
	ui.isLightPanelOpen    = config["UI.isLightPanelOpen"].as<bool>();
	ui.isMaterialPanelOpen = config["UI.isMaterialPanelOpen"].as<bool>();
	ui.isShaderPanelOpen   = config["UI.isShaderPanelOpen"].as<bool>();

}

void Config::save(std::string path)
{	
	YAML::Node config;

	config["Window.width"] = window.width;
	config["Window.height"] = window.height;
	config["Window.title"] = window.title;
	config["Window.fullscreen"] = window.fullscreen;

	config["UI.isCreditsPanelOpen"] = ui.isCreditsPanelOpen;
	config["UI.isLightPanelOpen"] = ui.isLightPanelOpen;
	config["UI.isMaterialPanelOpen"] = ui.isMaterialPanelOpen;
	config["UI.isShaderPanelOpen"] = ui.isShaderPanelOpen;

	std::ofstream fout(path);
	fout << config;
    fout.close();
}

// https://youtu.be/WbrDsnnC8dY?feature=shared