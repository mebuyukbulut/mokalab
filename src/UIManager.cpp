#include "UIManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imgui_internal.h"


#include "FileUtils.h"
#include "LightManager.h"
#include "Camera.h"
#include <iostream>

#include "EventDispatcher.h"
#include "Config.h"

#include "ParticleSystem.h"
#include "Entity.h"
#include "SceneManager.h"
#include "Builtin.h"

void UIManager::init(GLFWwindow* window, std::shared_ptr<Camera> camera) {
	_window = window;
    _camera = camera;

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

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
        

    // load config 
    isCreditsPanelOpen = config.ui.isCreditsPanelOpen;
    isLightPanelOpen = config.ui.isLightPanelOpen;
    isShaderPanelOpen = config.ui.isShaderPanelOpen;

}

void UIManager::terminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}

void UIManager::draw(SceneManager* sm ) {
	beginFrame();

	mainMenu();
    ps->onInspect();
	if(isShaderPanelOpen) shaderPanel();
	if(isCreditsPanelOpen) creditsPanel();

    //viewport_window();
    if (sm) {
        sm->onInspect();
        //sm->drawGizmo();
    }


    //ImGui::ShowDemoWindow();
	endFrame();
}


//
//void UIManager::viewport_window() {
//
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
//    ImGui::Begin("Viewport");// &viewport->get_active()); //ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_UnsavedDocument
//
//    //viewport->set_hovered(ImGui::IsWindowHovered());
//    ////std::cout << ImGui::IsWindowHovered() << std::endl;
//
//    auto t_size = ImGui::GetContentRegionAvail();//ImGui::GetWindowSize();
//    glm::ivec2 window_size{ t_size.x, t_size.y };
//
//    //if (window_size != viewport->get_resolution()) {
//    //    std::string res = "(" + std::to_string(window_size.x) + ", " + std::to_string(window_size.y) + ")";
//    //    //LogUtils::get().log("New window size: " + res);
//    //    viewport->set_resolution(window_size);
//    //    viewport->set_dirty(true);
//    //}
//    GLint defaultFBO = 0;
//    //glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &defaultFBO);
//    ImGui::Image(
//        (ImTextureID) defaultFBO,//viewport->texID(),
//        ImGui::GetContentRegionAvail(),
//        ImVec2(0, 1),
//        ImVec2(1, 0)
//    );
//
//
//    // viewport toolbar BEGIN
//    // https://gist.github.com/rmitton/f80cbb028fca4495ab1859a155db4cd8
//    float menuBarHeight = 25;
//    float toolbarSize = 30;
//    ImGuiWindow* viewport = ImGui::GetCurrentWindow();
//    //ImGuiViewport* viewport = ImGui::GetMainViewport();
//    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
//    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarSize));
//    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
//
//    ImGuiWindowFlags window_flags = 0
//        | ImGuiWindowFlags_NoDocking
//        | ImGuiWindowFlags_NoTitleBar
//        | ImGuiWindowFlags_NoResize
//        | ImGuiWindowFlags_NoMove
//        | ImGuiWindowFlags_NoScrollbar
//        | ImGuiWindowFlags_NoSavedSettings
//        ;
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
//    ImGui::Begin("TOOLBAR", NULL, window_flags);
//    ImGui::PopStyleVar();
//
//    ImGui::Button("Toolbar goes here", ImVec2(0, 37));
//
//    ImGui::End();
//    // viewport toolbar END
//
//
//    ImGui::End();
//    ImGui::PopStyleVar();
//
//
//}



bool UIManager::isHoverOnUI(){   
    return !_IsHoveringSceneViewport;
    //return ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow);
}

void UIManager::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    // dock directly to main window
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();
}
void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::mainMenu(){	
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { 
                Event e{ EventType::EngineExit, {} };
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                std::string filePath = FileUtils::openFileDialog();
                Event e{
                    EventType::ModelOpened,
                    std::make_unique<EventData_Text>(filePath)
                };
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {               
                std::string filePath = FileUtils::openFileDialog(L"",true);
                Event e{
                    EventType::SaveScene,
                    std::make_unique<EventData_Text>(filePath)
                };
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Load")) {                
                std::string filePath = FileUtils::openFileDialog();
                Event e{
                    EventType::LoadScene,
                    std::make_unique<EventData_Text>(filePath)
                };
                dispatcher.dispatch(e);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Shader Panel", nullptr, &isShaderPanelOpen)) {
                config.ui.isShaderPanelOpen = isShaderPanelOpen;
                config.save();
            }
            if(ImGui::MenuItem("Credits Panel", nullptr, &isCreditsPanelOpen)) {
                config.ui.isCreditsPanelOpen = isCreditsPanelOpen;
                config.save();
            }
            if(ImGui::MenuItem("Light Panel", nullptr, &isLightPanelOpen)) {
                config.ui.isLightPanelOpen = isLightPanelOpen;
                config.save();
            }
            ImGui::EndMenu();
        }
        

        if (ImGui::BeginMenu("Add")) {
            if (ImGui::BeginMenu("Add Light")) {
                if (ImGui::MenuItem("Add Point Light")) {
                    Event e{ 
                        EventType::AddLight,
                        std::make_unique<EventData_Text>(Builtin::LightType::Point)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Spot Light")) {
                    Event e{ 
                        EventType::AddLight,
                        std::make_unique<EventData_Text>(Builtin::LightType::Spot)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Direction Light")) {
                    Event e{ 
                        EventType::AddLight,
                        std::make_unique<EventData_Text>(Builtin::LightType::Directional)
                    };
                    dispatcher.dispatch(e);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Add Shape")) {
                if (ImGui::MenuItem("Add Cube")) {
                    Event e{ 
                        EventType::AddPrimitive,
                        std::make_unique<EventData_Text>(Builtin::Model::Cube)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Cone")) {
                    Event e{ 
                        EventType::AddPrimitive,
                        std::make_unique<EventData_Text>(Builtin::Model::Cube)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Cylinder")) {
                    Event e{ 
                        EventType::AddPrimitive,
                        std::make_unique<EventData_Text>(Builtin::Model::Cylinder)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Plane")) {
                    Event e{ 
                        EventType::AddPrimitive,
                        std::make_unique<EventData_Text>(Builtin::Model::Plane)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Torus")) {
                    Event e{ 
                        EventType::AddPrimitive,
                        std::make_unique<EventData_Text>(Builtin::Model::Torus)
                    };
                    dispatcher.dispatch(e);
                }
                if (ImGui::MenuItem("Add Monkey")) {
                    Event e{ EventType::AddMonkey };
                    dispatcher.dispatch(e);
                }
                ImGui::EndMenu();

            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void UIManager::shaderPanel()
{
    // Shader selection combo box
    const char* items[] = { "normal", "lambertian", "blinn-phong" };
    static int currentItem = 0;

    if (ImGui::Begin("Shader Controls")) {
        if (ImGui::Combo("Shader", &currentItem, items, IM_ARRAYSIZE(items))) {
            std::string itemStr = items[currentItem];
            Event e{ 
                EventType::ShaderSelected, 
                std::make_unique<EventData_Text>(itemStr)
            };
            dispatcher.dispatch(e);
        }
    }
    ImGui::End();
}

void UIManager::creditsPanel(){
    if (ImGui::Begin("Credits")) {
		ImGui::Text("Mokalab by Muhammet Esat BUYUKBULUT");
		ImGui::Text("@2025");

        //openmesh ekle 
    }
    ImGui::End();
}
