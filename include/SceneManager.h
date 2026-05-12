#pragma once
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "Scene.h"
#include "Entity.h"
#include "Transform.h"
#include "Material.h"
#include "RenderItem.h"

class Renderer;


class SceneManager : public Object
{
    Scene scene;
	std::string MWD; // Main Working Directory

	std::vector<std::unique_ptr<Entity>> _entities{};
    std::vector <Entity*> _selectedEntities{};
    Entity* _selectedEntity{};
    
    Renderer* _renderer{};
    class Camera* _camera{};
    class UIManager* _UI{};

    std::unique_ptr<class LightManager> _lightMng{};

    bool isScenePopupOpen = false;
    bool isViewportSelect = false; 
    bool isBoxSelectActive = false; 

    glm::vec2 mouseBeginPos{};
    glm::vec2 mouseEndPos{};
    glm::vec2 viewportPos; 
    glm::vec2 viewportPanelSize; 


    void collectRenderData(SceneRenderData& renderData);

public:
	SceneManager() = default;
    ~SceneManager() = default; 


    void initCommands();
    void initDefaults();

    void init(Renderer* renderer, Camera* camera, UIManager* UI);
    void draw();
    void updateMatrixRecursive(Entity* entity);


    Entity* getSelectedEntity() {
        //if (_selectedTransforms.empty()) return nullptr; 
        return _selectedEntity;
    }

    bool isSelected(Entity* entity);
    bool isLastSelected(Entity* entity);

    void select(Entity* entity);
    void deselect(Entity* entity);
    void deselectAll();


    void loadScene(std::string path);
    void saveScene();

    void drawHierarchyTreeRecursive(Entity* entity);
	void onInspect() override;
    void drawGizmo();

    // LIGHTS
    void addLight(LightType lightType);
    void sceneQuery(); // (Shader& shader);

    // SHAPES
	void addModel(std::string path, std::string entityName = "", bool loadAsync = false);
    
    void deleteSelected();
	void clearScene();

    std::string getUniqueName(std::string name);
    bool isUniqueName(std::string name);


    // Inherited via Object
    void serialize(YAML::Emitter& out) override;
    void deserialize(const YAML::Node& node) override;

};

