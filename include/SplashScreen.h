#pragma once
#include <filesystem>
#include <vector>
#include "PathResolver.h"

struct ProjectInfo{
    std::string name{};
    std::filesystem::path path{};

    bool isEmpty();
    //thumbnail
};
class SplashScreen
{
    struct GLFWwindow* _window{};

    PathResolver _pr{};
    std::vector<ProjectInfo> _allProjects{};
    ProjectInfo _selectedProject{}; 
    int _activePanel = 0; 
    bool _isExit = false; 

    void init();
    void terminate();
    void mainLoop();

    void startPanel(); 
    void newProjectPanel();
    void drawUI();

    void createProject(ProjectInfo pInfo);
    void startProject();

    std::vector<ProjectInfo> getProjects(); // read from project list 
    void setProjects(std::vector<ProjectInfo> projects); // write to project list

public:

    std::filesystem::path run();

};
