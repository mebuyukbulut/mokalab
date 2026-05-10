#include "SceneManager.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <yaml-cpp/yaml.h>

#include "Camera.h"
#include "Transform.h"
#include "EventDispatcher.h"
#include "IInspectable.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Shader.h"
#include "UIManager.h"

#include "Logger.h"
#include "AssetManager.h"
#include "RenderComponent.h"
#include "Builtin.h"
#include "Texture.h"
#include "FX.h"

FXRegistry fxReg{};

ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;

void SceneManager::collectRenderData(SceneRenderData &renderData)
{
    // collect lights 
    // light toplama işlemi sadece viewmode material için çalışsa yeterli olabilir. Ama şuan emin değilim.
	
    // for (const auto& entity : _entities) 
    //     if (Light* light = entity->getComponent<Light>()) 
	// 		renderData.lightItems.push_back(light);
            

    // collect render items respect to their index 
    for (uint32_t pickID = 0; pickID < _entities.size(); pickID++) 
    {
		Entity* entity = _entities[pickID].get();

        if (!entity->isActive()) continue;

        if(RenderComponent* renderComponent = entity->getComponent<RenderComponent>()){
            if (Model* model = renderComponent->_model.get()) {
                RenderItem item;
                item.model = model;
                item.transform = entity->transform->getGlobalMatrix();
                item.entityIndex = pickID;
                item.isSelected = entity->isSelected();
                renderData.renderItems.push_back(item);
            }
        }
        else if(Light* lightComponent = entity->getComponent<Light>()){
            LightItem item;
            item.light = lightComponent;
            item.transform = entity->transform->getGlobalMatrix();
            item.entityIndex = pickID;
            item.isSelected = entity->isSelected();
            renderData.lightItems.push_back(item);
        }

    }

    //std::cout << "renderData render items: " << renderData.renderItems.size() << "\t light items: " << renderData.lightItems.size() << std::endl;
}

void SceneManager::initCommands()
{
    dispatcher.subscribe(EventType::AddLight, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Text> t(static_cast<EventData_Text*>(e.release()));
        if(t->text == Builtin::LightType::Point)
            addLight(LightType::Point);
        else if(t->text == Builtin::LightType::Spot)
            addLight(LightType::Spot);
        else if(t->text == Builtin::LightType::Directional)
            addLight(LightType::Directional);
    });

    dispatcher.subscribe(EventType::AddPrimitive, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Text> t(static_cast<EventData_Text*>(e.release()));
        std::string pathStr = t->text;
        std::string::iterator beginPos = pathStr.begin() + pathStr.find_last_of(':') + 1 ; 
        std::string modelName = std::string(beginPos, pathStr.end());
        addModel(pathStr, modelName);
    });

    dispatcher.subscribe(EventType::AddMonkey, [&](std::unique_ptr<EventData> e) {
        std::filesystem::path AssetRoot = std::filesystem::current_path().parent_path() / "assets";
        auto model = AssetRoot / "models/monkey/monkey.obj";
        addModel(model.c_str(), "Monkey", true);
    });

    dispatcher.subscribe(EventType::Delete, [&](std::unique_ptr<EventData> e) {
        deleteSelected();
    });
    dispatcher.subscribe(EventType::Select, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Point> p(static_cast<EventData_Point*>(e.release()));
        mousePos = glm::vec2(p->vec.x, p->vec.y);
        isViewportSelect = true;
    });

    dispatcher.subscribe(EventType::ScenePopup, [&](std::unique_ptr<EventData> e) {
        isScenePopupOpen = true;
    });
    dispatcher.subscribe(EventType::SaveScene, [&](std::unique_ptr<EventData> e) {
        saveScene();
    });
    dispatcher.subscribe(EventType::LoadScene, [&](std::unique_ptr<EventData> e) {
        loadScene("");
    });


    dispatcher.subscribe(EventType::ModelOpened, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Text> t(static_cast<EventData_Text*>(e.release()));
        std::string modelPath = t->text;
        addModel(modelPath, "", true);
    });

    dispatcher.subscribe(EventType::FocusToSelectedObject, [&](std::unique_ptr<EventData> e) {
        if(Entity* selectedEntity = getSelectedEntity())
            _camera->resetFrame(selectedEntity->transform.get());
    });

    dispatcher.subscribe(EventType::MouseDrag, [&](std::unique_ptr<EventData> e){
        std::unique_ptr<EventData_DoublePoint> points ( static_cast<EventData_DoublePoint*>(e.release()));
        LOG_TRACE("vecA x: {}\ty:{}\tz:{}",points->vecA.x, points->vecA.y, points->vecA.z);
        LOG_TRACE("vecB x: {}\ty:{}\tz:{}",points->vecB.x, points->vecB.y, points->vecB.z);
    });

}
void SceneManager::initDefaults()
{
    // Load default Material
    for(const char* path : Builtin::Material::All)
        g_Assets.get<Material>(path); 

    // Load default Models
    for(const char* key : Builtin::Model::All)
        g_Assets.get<Model>(key);

    std::filesystem::path AssetRoot = std::filesystem::current_path().parent_path() / "assets";

    // Load icons
    for(const char* key : Builtin::Icon::All){
        TextureSettings ts;
        auto iconPath = AssetRoot / ("icons/" + std::string(key) + ".png");
        LOG_TRACE("{}", iconPath.c_str());
        ts.realPath = iconPath.c_str();
        g_Assets.get<Texture>(key, &ts, false); // data is not unique ptr so cannot be async
    }
    

    auto model = AssetRoot / "models/monkey/monkey.obj";
    
    g_Assets.get<Model>(model.c_str(), nullptr, true);
    //g_Assets.get<Model>(MWD + "/../assets/models/monkey/monkey.obj", nullptr, true);
}
void SceneManager::init(Renderer* renderer, Camera* camera, UIManager* UI) {
    fxReg.init();
	MWD = std::filesystem::current_path().string();
    
    _renderer = renderer;
    _renderer->fxReg = &fxReg;
    _camera = camera;
    _UI = UI;

	_lightMng = std::make_unique<LightManager>();
    
    initCommands();
    initDefaults();
}



void SceneManager::draw() {
	ViewMode viewMode = _renderer->getViewMode();

	// update all global matrices
    for (const auto& entity : _entities) 
        if (entity->transform->isRoot()) // && entity->getComponent<Model>()
            updateMatrixRecursive(entity.get());
        
    SceneRenderData renderData{};
    collectRenderData(renderData);
    _lightMng->queryLights(renderData.lightItems);





    // mouse click ile ekranda öge yakalama
    if (isViewportSelect && ImGuizmo::IsOver())
        isViewportSelect = false;

    // calculate mouse position relative to viewport
    glm::vec2 mPos = glm::vec2(mousePos.x, mousePos.y);
    glm::vec2 panelPos = glm::vec2(viewportPos.x, viewportPos.y);
    glm::vec2 panelSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
    mPos = mPos - panelPos;
    mPos.y = panelSize.y - mPos.y;

    _renderer->renderScene(renderData, isViewportSelect, mPos);

    if (isViewportSelect) {
		// get ID from framebuffer and object selection: 
        uint32_t selectedID = _renderer->getSelection(mPos);
        LOG_TRACE("selection UUID: {}", std::to_string(selectedID));

        if (selectedID != 0)
        {
            selectedID -= 1; // because we added +1 when drawing
            if (!ImGui::GetIO().KeyCtrl)
                deselectAll();
            select(_entities[selectedID].get());
		}
        else
            deselectAll();

        isViewportSelect = false;
    }

}


void SceneManager::updateMatrixRecursive(Entity* entity)
{
    entity->transform->getGlobalMatrix();

    for (Transform* i : entity->transform->getChildren())
        updateMatrixRecursive(i->owner);
}




bool SceneManager::isSelected(Entity* entity){
    return std::find(_selectedEntities.begin(), _selectedEntities.end(), entity) != _selectedEntities.end();
}

bool SceneManager::isLastSelected(Entity* entity){
    return entity == _selectedEntity;
}

void SceneManager::select(Entity* entity){
    if (!isSelected(entity))
        _selectedEntities.push_back(entity);

    _selectedEntity = entity;
    entity->select(); 
}
void SceneManager::deselect(Entity* entity)
{
    if (!isSelected(entity))
        return; 
    _selectedEntities.erase(std::find(_selectedEntities.begin(), _selectedEntities.end(), entity));
    _selectedEntity = nullptr;
    entity->deselect();

    if (!_selectedEntities.empty())
        _selectedEntity = _selectedEntities.back();

}
void SceneManager::deselectAll(){
    for(auto& i : _selectedEntities)
        i->deselect();
    _selectedEntities.clear();
    _selectedEntity = nullptr;
}



void SceneManager::loadScene(std::string path) {
    LOG_TRACE("Loading scene...");

    // 1. Aşama tüm entity'leri oluşturma 
	path = "save0.yaml";
	clearScene(); // delete all entities first
    YAML::Node root = YAML::LoadFile(path);
    deserialize(root);

    LOG_TRACE("Scene was loaded.");




}
void SceneManager::saveScene() {
    LOG_TRACE("Scene is saving");
    YAML::Emitter out;
    serialize(out);

    std::string path = "save0.yaml";
    std::ofstream fout(path);
    fout << out.c_str();
    LOG_INFO("Scene saved");
}

/// Call recursively to populate each level of children
void SceneManager::drawHierarchyTreeRecursive(Entity* entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected(entity)) flags |= ImGuiTreeNodeFlags_Selected;
    if (entity->transform->getChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;

    bool opened = false;

    if (isLastSelected(entity)) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(230, 125, 15, 255)); // orange foreground
        //opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, entity->name.c_str());
        opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, "%s", entity->name.c_str());
        //TreeNodeEx((void*)(uintptr_t)entity->UUID, flags, "%s", entity->name.c_str());
        ImGui::PopStyleColor(1);
    }
    else {
        opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity->transform->UUID, flags, "%s", entity->name.c_str());
    }


    // Seçme işlemi
    if (ImGui::IsItemClicked()){
        if (!ImGui::GetIO().KeyCtrl)
            deselectAll();
        
        select(entity);
    }


    // --- BURASI: DRAG SOURCE (Sürüklemeyi Başlat) ---
    if (ImGui::BeginDragDropSource()) {
        // Sürüklenen nesnenin pointer'ını paketle (Payload)
        ImGui::SetDragDropPayload("ENTITY_HIERARCHY_NODE", &entity, sizeof(Entity*));

        // Sürüklerken farenin yanında ne görünsün?
        ImGui::Text("Moving: %s", entity->name.c_str());
        ImGui::EndDragDropSource();
    }

    // --- BURASI: DRAG TARGET (Üzerine Bırakmayı Kabul Et) ---
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY_NODE")) {
            // Paketi aç (Sürüklenen Entity'yi al)
            Entity* draggedEntity = *(Entity**)payload->Data;

            // Kendi kendine veya zaten parent'ı olan birine bırakılmadığından emin ol
            if (draggedEntity != entity) {
                draggedEntity->transform->setParent(entity->transform.get()); 
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Recursive çağrı
    if (opened) {
        for (Transform* transform : entity->transform->getChildren())
            drawHierarchyTreeRecursive(transform->owner);
    
        ImGui::TreePop();
    }
}

void SceneManager::onInspect()
{
    // HIERARCHY PANEL

    ImGuiTreeNodeFlags rootFlag =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Bullet |
        ImGuiTreeNodeFlags_Leaf;

    ImGui::Begin("Scene");

    if (ImGui::IsItemClicked())
        LOG_TRACE("Scene window tab was clicked");


    if (ImGui::TreeNodeEx("root", rootFlag)){
        if (ImGui::IsItemClicked())
            LOG_TRACE("Scene root was clicked");
        
        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(55, 55, 55, 255)); // gray background
        
        for (const auto& i : _entities) 
            if(i->transform->isRoot())
                drawHierarchyTreeRecursive(i.get()); // Call recursively to populate each level of children

        ImGui::PopStyleColor(1);

        ImGui::TreePop();  // This is required at the end of the if block
    }

    // drawHierarchy Tree döngüsünün dışına, Begin/End arasına:
    ImGui::Dummy(ImGui::GetContentRegionAvail()); // Boş bir alan yarat
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY_NODE")) {
            Entity* draggedEntity = *(Entity**)payload->Data;
            draggedEntity->transform->setParent(nullptr); // Bağımsız yap
        }
        ImGui::EndDragDropTarget();
    }


    // bos alana tiklamayi yakala:
    if (ImGui::IsWindowFocused() &&
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered()
        )
    {
        deselectAll();        
    }


    ImGui::End();


    // FX
    fxReg.onInspect();

    // PROPERTIES PANEL

    ImGui::Begin("Properties");
    if(_selectedEntity)
		_selectedEntity->onInspect();
    ImGui::End();


    // DEBUG PANEL
    ImGui::Begin("Debug Window");

    ImVec2 debugPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)(intptr_t)_renderer->getDebugImage(),
        debugPanelSize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();




    // TERMINAL PANEL
    ImGui::Begin("TERMINAL");
    //static bool terminalButton_state_all = true;
    static bool states[7]{true,true,true,true,true,true,true};
    static std::string state_strs[7]{"Trace","Debug","Info","Warning","Error","Success","Critical"};
    bool& terminalButton_state_trace    = states[0];
    bool& terminalButton_state_debug    = states[1];
    bool& terminalButton_state_info     = states[2];
    bool& terminalButton_state_warning  = states[3];
    bool& terminalButton_state_error    = states[4];
    bool& terminalButton_state_success  = states[5];
    bool& terminalButton_state_critical = states[6];

    for(int i = 0; i < 7; i++){
        if (states[i]) {
            // Buton seçiliyken Lime olsun
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        } else{
            // Buton seçili değilken Gray olsun
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,        ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        if (ImGui::Button(state_strs[i].c_str())) states[i] = !states[i];       
        if (i != 6) ImGui::SameLine();
                
        ImGui::PopStyleColor(3);
    }


    // Kaydırma alanı için bir Child Window açıyoruz
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    auto& logs = Logger::get().getMessages();
    
    // ekranda gösterilmeyecek olan mesajları filtreliyoruz.   
    static std::vector<int> filteredIndices;
    filteredIndices.clear();
    filteredIndices.reserve(logs.capacity());

    for (int i = 0; i < logs.size(); i++) {
        char prefix = logs[i][1]; // [T], [D], [I] gibi...
        
        // Filtreleme mantığı
        if (prefix == 'T' && !states[0]) continue;
        if (prefix == 'D' && !states[1]) continue;
        if (prefix == 'I' && !states[2]) continue;
        if (prefix == 'W' && !states[3]) continue;
        if (prefix == 'E' && !states[4]) continue;
        if (prefix == 'S' && !states[5]) continue;
        if (prefix == 'C' && !states[6]) continue;
        
        filteredIndices.push_back(i);
    }

    // Clipper nesnesini oluşturuyoruz
    ImGuiListClipper clipper;
    clipper.Begin(filteredIndices.size()); // Toplam log sayısı

    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            int logIndex = filteredIndices[i];
            const auto& message = logs[logIndex];
            
            // Renklendirme (Daha önceki LogEntry yapısını kullandığını varsayıyorum)
            ImVec4 color = ImColor(255, 0, 0, 255).Value;
            
            char prefix = message[1];
            if      (prefix == 'T') color = ImColor(128, 128, 128).Value;
            else if (prefix == 'D') color = ImColor(  0, 255, 255).Value;
            else if (prefix == 'I') color = ImColor(255, 255, 255).Value;
            else if (prefix == 'W') color = ImColor(255, 255,   0).Value;
            else if (prefix == 'E') color = ImColor(255,   0,   0).Value;
            else if (prefix == 'S') color = ImColor( 50, 205,  50).Value;
            else if (prefix == 'C') color = ImColor(255,   0, 255).Value;
            else                    color = ImColor(  0,   0,   0).Value; // fallback

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(message.c_str());
            ImGui::PopStyleColor();
        }
    }
    
    // Otomatik en alta kaydırma (opsiyonel)
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();







    // VIEWPORT PANEL

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");// &viewport->get_active()); //ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_UnsavedDocument
    _UI->setHoverOnUI(ImGui::IsWindowHovered());
    //viewport->set_hovered(ImGui::IsWindowHovered());
    ////std::cout << ImGui::IsWindowHovered() << std::endl;
    

    //ImGui::OpenPopupOnItemClick("SceneContextMenu", ImGuiPopupFlags_None);
    
    if (isScenePopupOpen) {
        isScenePopupOpen = false;
        ImGui::OpenPopup("SceneContextMenu");
    }
    
    if (ImGui::BeginPopupContextWindow("SceneContextMenu", ImGuiPopupFlags_::ImGuiPopupFlags_None)) // sağ tıkla pencere boşluğuna tıklanırsa
    {
        if (ImGui::BeginMenu("Add Light")) {
            if (ImGui::MenuItem("Add Point Light")) {
                Event e{ 
                    EventType::AddLight,
                    std::make_unique<EventData_Text>(Builtin::LightType::Point)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Spot Light")) {
                Event e{ 
                    EventType::AddLight,
                    std::make_unique<EventData_Text>(Builtin::LightType::Spot)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Direction Light")) {
                Event e{ 
                    EventType::AddLight,
                    std::make_unique<EventData_Text>(Builtin::LightType::Directional)};
                dispatcher.dispatch(e);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add Shape")) {
            if (ImGui::MenuItem("Add Cube")) {
                Event e{ 
                    EventType::AddPrimitive,
                    std::make_unique<EventData_Text>(Builtin::Model::Cube)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Cone")) {
                Event e{ 
                    EventType::AddPrimitive,
                    std::make_unique<EventData_Text>(Builtin::Model::Cone)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Cylinder")) {
                Event e{ 
                    EventType::AddPrimitive,
                    std::make_unique<EventData_Text>(Builtin::Model::Cylinder)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Plane")) {
                Event e{ 
                    EventType::AddPrimitive,
                    std::make_unique<EventData_Text>(Builtin::Model::Plane)};
                dispatcher.dispatch(e);
            }
            if (ImGui::MenuItem("Add Torus")) {
                Event e{ 
                    EventType::AddPrimitive,
                    std::make_unique<EventData_Text>(Builtin::Model::Torus)};
                dispatcher.dispatch(e);
            }
            ImGui::EndMenu();

        }


        ImGui::Separator();

        ImGui::EndPopup();
    }



    auto t_size = ImGui::GetContentRegionAvail();//ImGui::GetWindowSize();
    glm::ivec2 window_size{ t_size.x, t_size.y };

    //if (window_size != viewport->get_resolution()) {
    //    std::string res = "(" + std::to_string(window_size.x) + ", " + std::to_string(window_size.y) + ")";
    //    //LogUtils::get().log("New window size: " + res);
    //    viewport->set_resolution(window_size);
    //    viewport->set_dirty(true);
    //}
     
     
     
    //GLint defaultFBO = 0;
    ////glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &defaultFBO);
    //ImGui::Image(
    //    (ImTextureID)defaultFBO,//viewport->texID(),
    //    ImGui::GetContentRegionAvail(),
    //    ImVec2(0, 1),
    //    ImVec2(1, 0)
    //);

    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    viewportPanelSize = glm::vec2(panelSize.x, panelSize.y);
    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos(); 
    viewportPos = glm::vec2(cursorScreenPos.x, cursorScreenPos.y);
    ImGui::Image((ImTextureID)(intptr_t)_renderer->getViewportImage(),
        panelSize, ImVec2(0, 1), ImVec2(1, 0));

    // viewport toolbar BEGIN
    // https://gist.github.com/rmitton/f80cbb028fca4495ab1859a155db4cd8
    float menuBarHeight = 25;
    float toolbarSize = 34;
    ImGuiWindow* viewport = ImGui::GetCurrentWindow();
    //ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarSize));
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;



    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("TOOLBAR", NULL, window_flags);

    ImVec4 tint = false
    ? ImVec4(0.4f, 0.7f, 1.0f, 1.0f)
    : ImVec4(1, 1, 1, 1);

    
    if (ImGui::ImageButton(
            "t",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::EditorTool::Translate)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    }    
    ImGui::SameLine();
    if (ImGui::ImageButton(
            "r",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::EditorTool::Rotate)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    } 
    ImGui::SameLine();   
    if (ImGui::ImageButton(
            "s",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::EditorTool::Scale)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    }
    ImGui::SameLine();
	
    ImGui::Dummy(ImVec2(0, 240)); // boşluk olsun 
    ImGui::SameLine();

    if (ImGui::ImageButton(
            "p",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::ViewMode::Lit)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        _renderer->setViewMode(ViewMode::Material);
    }
    ImGui::SameLine();

    if (ImGui::ImageButton(
            "m",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::ViewMode::Matcap)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        _renderer->setViewMode(ViewMode::Matcap);
    }
    ImGui::SameLine();

    if (ImGui::ImageButton(
            "w",
            (ImTextureID)(intptr_t)g_Assets.get<Texture>(Builtin::Icon::ViewMode::Wireframe)->getId(),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), tint))
    {
        _renderer->setViewMode(ViewMode::Wireframe);
    }



    // if(ImGui::Button("T", ImVec2(0, 37))) // translate 
    //     mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    // ImGui::SameLine();
    // if (ImGui::Button("R", ImVec2(0, 37))) // rotate
    //     mCurrentGizmoOperation = ImGuizmo::ROTATE;
    // ImGui::SameLine();
    // if (ImGui::Button("S", ImVec2(0, 37))) // scale
    //     mCurrentGizmoOperation = ImGuizmo::SCALE;
    // //ImGui::Button("Toolbar goes here", ImVec2(0, 37));

    // ImGui::SameLine();
	// ImGui::Dummy(ImVec2(0, 240)); // boşluk olsun 

    // ImGui::SameLine();
	// // view mode buttons
    // if (ImGui::Button("P", ImVec2(0, 37))) // Solid (material) view 
    //     _renderer->setViewMode(ViewMode::Material);
    // ImGui::SameLine();
    // if (ImGui::Button("M", ImVec2(0, 37))) // Matcap view
    //     _renderer->setViewMode(ViewMode::Matcap);
    // ImGui::SameLine();
    // if (ImGui::Button("W", ImVec2(0, 37))) // Wireframe view
	// 	_renderer->setViewMode(ViewMode::Wireframe);

    ImGui::End();
    ImGui::PopStyleVar();


    _renderer->resizeViewport(panelSize.x, panelSize.y);
    // viewport toolbar END

    drawGizmo();

    ImGui::End();
    ImGui::PopStyleVar();
}
void SceneManager::drawGizmo()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();


    // Viewport boyutu
    float windowWidth = ImGui::GetWindowSize().x;
    float windowHeight = ImGui::GetWindowSize().y;

    // Küçük bir alan ayır (örneğin 128x128 px)
    float gizmoSize = 100.0f;
    ImVec2 gizmoPos = ImVec2(windowWidth-gizmoSize-1, 80); // sağ üst köşe

    

    // Kamera matrislerini kullan
    const float* view = glm::value_ptr(_camera->getViewMatrix());
    const float* projection = glm::value_ptr(_camera->getProjectionMatrix());


    // ImGuizmo’nun çizim bölgesini ayarla
    ImGuizmo::SetRect(gizmoPos.x, gizmoPos.y, gizmoSize, gizmoSize);
    //ImGuizmo::DrawCubes(view, projection, nullptr, 0);

    //float a = view[0];

    ImGuizmo::ViewManipulate(
        (float*)view,
        10.0f,                // manipülasyon boyutu
        gizmoPos,
        ImVec2(gizmoSize, gizmoSize),
        0x79000000            // opsiyonel arka plan rengi
    );
    //float b = view[0];
    //if(a!= b)
    //    std::cout << "a: " << a << "\tb: "<<b << std::endl;


    if (!_selectedEntity) return;

    // Make sure to call inside ImGui frame:
    ImGuizmo::BeginFrame();

    // Get viewport size
    ImVec2 windowPos = ImGui::GetWindowPos();
    auto windowSize = ImGui::GetWindowSize(); //_camera->getWindowSize();

    // Setup ImGuizmo rect
    //ImGuizmo::SetDrawlist(); // bu olmadan gizmo gozukmuyor
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);


    glm::mat4 modelMatrix = _selectedEntity->transform->getGlobalMatrix();
    glm::vec3 selectedPos = _selectedEntity->transform->getPosition();

    bool gizmoUsed = ImGuizmo::Manipulate(
        glm::value_ptr(_camera->getViewMatrix()),
        glm::value_ptr(_camera->getProjectionMatrix()),
        mCurrentGizmoOperation,
        ImGuizmo::WORLD,
        glm::value_ptr(modelMatrix)
    );

    if (gizmoUsed){
        _selectedEntity->transform->setLocalMatrix(modelMatrix);

        if(mCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE){
            glm::vec3 deltaPos = _selectedEntity->transform->getPosition() - selectedPos;
            for(Entity* e : _selectedEntities){
                if(e == _selectedEntity) continue;
                    e->transform->move(deltaPos);
            }

        }
        

    }

}


void SceneManager::addLight(LightType lightType)
{
    auto light = _lightMng->createLight(lightType);
    if (!light) {
        LOG_ERROR("SceneManager: Failed to create light");
        return;
	}
	auto entity = std::make_unique<Entity>();
	entity->name = light->name;
    entity->addComponent(std::move(light));
	_entities.push_back(std::move(entity));
}

void SceneManager::sceneQuery()//(Shader& shader)
{

}

void SceneManager::addModel(std::string path, std::string entityName, bool loadAsync)
{
    auto entity = std::make_unique<Entity>();

    if (!entityName.empty())
        entity->name = getUniqueName(entityName);
    else {
        unsigned int slashIndex = path.find_last_of('/');
        unsigned int pointIndex = path.find_last_of('.');
        std::string directory = path.substr(0, slashIndex);
        std::string modelName = path.substr(slashIndex+1, pointIndex-slashIndex -1);
		entity->name = getUniqueName(modelName);
    }


    auto renderComponent = std::make_unique<RenderComponent>();
    renderComponent->_model = g_Assets.get<Model>(path, nullptr, loadAsync);
    entity->addComponent(std::move(renderComponent));

    _entities.push_back(std::move(entity));
}




void SceneManager::deleteSelected()
{
    if (!_selectedEntity) return;

    _entities.erase(std::remove_if(_entities.begin(), _entities.end(),
        [this](const std::unique_ptr<Entity>& t) {
            return t.get() == _selectedEntity;
		}), 
        _entities.end());

	_selectedEntities.erase(std::find(_selectedEntities.begin(), _selectedEntities.end(), _selectedEntity));

    _selectedEntity = nullptr;

    //_entities.remove_if([this](const std::unique_ptr<Entity>& t) {
    //    return t.get() == _selectedEntity;
    //    });

    //_selectedEntities.remove_if([this](Entity* t) {
    //    return t == _selectedEntity;
    //    });
}

void SceneManager::clearScene()
{
    _selectedEntity = nullptr;
    _selectedEntities.clear();
    _entities.clear();
}



std::string SceneManager::getUniqueName(std::string name)
{
    int i{};
    std::string uniq_name = name; 
    while (!isUniqueName(uniq_name))    
        uniq_name = name + std::to_string(i++);
    
    return uniq_name;
}

bool SceneManager::isUniqueName(std::string name)
{
    for (const auto& entity : _entities)
        if (entity->name == name) return false;

    return true;
}

void SceneManager::serialize(YAML::Emitter& out)
{
    out << YAML::BeginDoc;
	out << YAML::BeginMap;

    out << YAML::Key << "Scene" << YAML::Value << "istanbul";
    out << YAML::Key << "Version" << YAML::Value << "1.0";
    out << YAML::Key << "Entities" << YAML::Value;

    out << YAML::BeginSeq;
    for (const auto& entity : _entities) {
        entity->serialize(out);
    }
    out << YAML::EndSeq;

	out << YAML::EndMap;
    out << YAML::EndDoc;
}

void SceneManager::deserialize(const YAML::Node& node)
{
    auto sceneNameNode = node["Scene"];
    std::string scene_name = sceneNameNode.as<std::string>();

    Event windowTitleTextEvent{
        EventType::SetMainWindowTitle, 
        std::make_unique<EventData_Text>("Model Viewer - " + scene_name)
    };
    dispatcher.dispatch(windowTitleTextEvent);

    auto versionNode = node["Version"]; // dosya versiyonu


    // UUID -> Entity* eşleşmesi için geçici bir "adres defteri"
    std::unordered_map<uint64_t, Transform*> entityMap;
    auto entitiesNode = node["Entities"];

    for (const auto& entityNode : entitiesNode) {
        auto entity = std::make_unique<Entity>();
        entity->deserialize(entityNode);
        LOG_TRACE("Load entity: {}", entity->name);

        // parent child relationship için transformun uuid sini mapliyoruz.
        uint64_t id = entity->transform->UUID;
        entityMap[id] = entity->transform.get();

        _entities.emplace_back(std::move(entity));
    }




    // --- PASS 2: Aile Bağlarını (Parent-Child) Kur ---
    for (const auto& entityNode : entitiesNode) {
        auto transformNode = entityNode["transform"];
        if (transformNode["parentUUID"]) { // YAML'da bu anahtarı kaydettiğini varsayıyoruz
            uint64_t childID = transformNode["UUID"].as<uint64_t>();
            uint64_t parentID = transformNode["parentUUID"].as<uint64_t>();

            if (parentID != 0) { // 0 genelde "root" (ebeveynsiz) demektir
                Transform* child = entityMap[childID];
                Transform* parent = entityMap[parentID];

                if (child && parent) {
                    child->setParent(parent);
                    LOG_TRACE("Pass 2: Linked {} -> Parent: {}", child->owner->name, parent->owner->name);
                }
            }
        }
    }


}

