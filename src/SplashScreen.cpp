#include "SplashScreen.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imgui_internal.h"

#include "FileUtils.h"
#include "StringUtils.h"
#include "Logger.h"



void SplashScreen::init()
{
    // glfw window creation
    // --------------------
    _window = glfwCreateWindow(
        800, 500, 
        "MOKALAB - PROJECT SELECTION",
        NULL, NULL);

    if (_window == NULL)
    {
        LOG_CRITICAL("Failed to create GLFW window");
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(_window);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
	if (!gladLoadGL(glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize OpenGL context\n");
		std::exit(EXIT_FAILURE);
	}    

    
    // INIT IMGUI
    // ---------------------------------------

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();


    ImGuiIO& io = ImGui::GetIO();
    // Türkçe karakter aralığını ekleyelim
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Latin + Latin Supplement
        0x0100, 0x017F, // Latin Extended-A (Türkçe karakterler burada)
        0
    };
    //// Yeni font yükleme (örnek: Roboto)
    //io.Fonts->AddFontFromFileTTF("fonts/Roboto-VariableFont_wdth,wght.ttf", 16.0f, NULL, ranges);

    // Eğer default fontu da korumak istersen:
    io.FontDefault = io.Fonts->AddFontFromFileTTF("../assets/fonts/Roboto-VariableFont_wdth,wght.ttf", 16.0f, NULL, ranges);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    //io.ConfigWindowsResizeFromEdges = true;
    //io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 410");



    // INIT other things
    // ---------------------------------------
    _allProjects = getProjects();
}

void SplashScreen::terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
}

void SplashScreen::mainLoop()
{
    while (!glfwWindowShouldClose(_window))
    {        
        drawUI();

        glfwSwapBuffers(_window);
        glfwPollEvents();

        if(_isExit) break; 
    }
}

void SplashScreen::startPanel()
{		
    ImGui::Text("PROJECT LIST");      

    static int selectedIndex = -1;
    //std::vector<std::string> vecStr{"Opt1", "Opt2", "Opt3"};


    // scrollable area
    ImGui::BeginChild("fx_list", ImVec2(0, 200), true);
    for (int i = 0; i < _allProjects.size(); i++)
    {
        ImGui::PushID(i);

        // // select row
        // bool selected = (selectedIndex == i);
        float total_height = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight();
        if (ImGui::Selectable("##row", false/*selected*/, ImGuiSelectableFlags_AllowOverlap, ImVec2(0, total_height+4)))
        {
            selectedIndex = i;
        }

        ImGui::SameLine();

        
        if(ImGui::Button("Run", ImVec2(0,total_height))){
            _selectedProject = _allProjects[i];
            startProject();
        }
        //ImGui::Checkbox("##enabled", &selected);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("%s", _allProjects[i].name.c_str());
        ImGui::Text("%s", _allProjects[i].path.c_str());
        ImGui::EndGroup();

        // ImGui::SameLine(280);//(200);

        // if (ImGui::SmallButton("X"))
        // {   
        //     if(selectedIndex == i) selectedIndex--;
        //     vecStr.erase(vecStr.begin() + i);
        //     ImGui::PopID();
        //     break;
        // }

        // for(FXParam& param : parameters)
        //     param.onInspect();


        ImGui::PopID();
    }

    ImGui::EndChild();

}

void SplashScreen::newProjectPanel()
{
    static char projNameBuf[128]; 
    static char projPathBuf[512]; 

    ImGui::Text("NEW PROJECT");   
    ImGui::Text("Name:");      
    ImGui::InputText("##Name", projNameBuf, std::ssize(projNameBuf)); //, ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsHexadecimal);

    ImGui::Text("Location:");   
    ImGui::SetItemTooltip("The location where the project folder would be created");
    ImGui::InputText("##Location", projPathBuf, std::ssize(projPathBuf));
    ImGui::SameLine();
    if(ImGui::Button("...")){
        std::string str = FileUtils::openFileDialog(L"", false, true);    
        std::snprintf(projPathBuf, sizeof(projPathBuf), "%s", str.c_str());
    }    
    
    if(ImGui::Button("Create")){
        ProjectInfo pInfo; 
        pInfo.name = std::string(projNameBuf);
        pInfo.path = std::filesystem::path(projPathBuf)/projNameBuf;
        createProject(pInfo);

        _selectedProject = pInfo;
        startProject();
    }

}

void SplashScreen::drawUI()
{    
    // BEGIN FRAME
    // ---------------------------------------
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // dock directly to main window
    //ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();


    // CONTEXT
    // ---------------------------------------
    
    static bool layout_initialized = false;

    // Ana Viewport ve DockSpace oluşturma
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                         ImGuiWindowFlags_NoDocking |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("MainDockSpaceWindow", nullptr, host_window_flags);
    ImGuiID dockspace_id = ImGui::GetID("MyMainDockSpace");

    // DÜZELTME 1: DOCKSPACE BAYRAKLARI (KİLİTLEME VE BAŞLIK GİZLEME)
    ImGuiDockNodeFlags dockspace_flags = (ImGuiDockNodeFlags)ImGuiDockNodeFlags_NoTabBar |         // Tab / Başlık çubuğunu tamamen gizler
                                         ImGuiDockNodeFlags_NoResize |         // Pencereler arasındaki ayırıcı çizgiyi kilitler
                                         ImGuiDockNodeFlags_NoDockingOverMe;  // Başka pencerenin buraya dock edilmesini engeller
                                         //ImGuiDockNodeFlags_NoDockingSplitMe;  // Pencerelerin sürüklenip sökülmesini/bölünmesini engeller
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags); //ImGuiDockNodeFlags_None);

    // Layout daha önce oluşturulmadıysa ilk karede (frame) otomatik böl
    if (!layout_initialized) {
        layout_initialized = true;

        // Mevcut düzeni temizle ve yeni bir kök dü düğümü oluştur
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        // Ana alanı soldan %25 oranında böl (Sol Panel ve Sağ Panel)
        ImGuiID dock_left_id;
        ImGuiID dock_right_id;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.30f, &dock_left_id, &dock_right_id);

        // // Sağ alanı da alt kısımdan %30 oranında böl (Sağ Üst ve Alt Konsol)
        // ImGuiID dock_bottom_id;
        // dock_right_id = ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.30f, &dock_bottom_id, nullptr);

        // Pencereleri isimlerine göre ilgili dock düğümlerine atayın
        ImGui::DockBuilderDockWindow("MML", dock_left_id);
        ImGui::DockBuilderDockWindow("MMR", dock_right_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }
    ImGui::End();

    // DÜZELTME 2: PENCERE BAYRAKLARI
    ImGuiWindowFlags child_window_flags = ImGuiWindowFlags_NoTitleBar | 
                                          ImGuiWindowFlags_NoCollapse | 
                                          ImGuiWindowFlags_NoResize | 
                                          ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("MML", nullptr, child_window_flags)) {
        
        if(ImGui::Button("START")){
            _activePanel = 0; 
        }
        if(ImGui::Button("NEW PROJECT")){
            _activePanel = 1;             
        }

    }
    ImGui::End();

    if (ImGui::Begin("MMR", nullptr, child_window_flags)) {
        if(_activePanel == 0) startPanel();
        else if(_activePanel == 1) newProjectPanel();
    }
    ImGui::End();
    

    // END FRAME
    // ---------------------------------------
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SplashScreen::createProject(ProjectInfo pInfo)
{
    std::filesystem::path dirPath = pInfo.path;
    FileUtils::createDirectory(dirPath);

    std::string mokaContext = pInfo.name + "\n" + "version 1.0";
    FileUtils::writeFile(dirPath / "project.moka", mokaContext);
    
    std::string pConfig = 
    "UI.isCreditsPanelOpen: false\nUI.isLightPanelOpen: false\nUI.isMaterialPanelOpen: true\nUI.isShaderPanelOpen: false";
    FileUtils::writeFile(dirPath / "config.yaml", pConfig);

    FileUtils::createDirectory(dirPath / "content");

    _allProjects.insert(_allProjects.begin(), pInfo);
    setProjects(_allProjects);
}

void SplashScreen::startProject()
{
    _isExit = true; 
}

// appData/last-projects-list.txt son açılan projeleri okur 
std::vector<ProjectInfo> SplashScreen::getProjects()
{
    std::string lastProjectsPath = _pr.lastProjectsPath.c_str();

    if(!FileUtils::isExists(lastProjectsPath)){
        FileUtils::writeFile(lastProjectsPath, "");        
    }
    std::string lastProjectsList = FileUtils::readFile(lastProjectsPath);

    std::vector<ProjectInfo> projects{};
    for(auto path : StringUtils::split(lastProjectsList, '\n')){
        ProjectInfo info{};
        info.path = path;   
        info.name = info.path.filename();     
        projects.push_back(info);
    }

    return projects;
}

// appData/last-projects-list.txt son açılan projeleri yazar 
void SplashScreen::setProjects(std::vector<ProjectInfo> projects)
{
    std::string lastProjectsPath = _pr.lastProjectsPath.c_str();

    std::string str{};
    for(auto i : projects)
        str += i.path.c_str(), str += "\n";

    FileUtils::writeFile(lastProjectsPath, str); 
}

std::filesystem::path SplashScreen::run()
{
    init();
    mainLoop();
    terminate();

    return _selectedProject.path;
}
