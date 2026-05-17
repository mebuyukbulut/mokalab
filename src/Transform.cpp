#include "Transform.h"
#include <imgui.h>
#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "YAMLHelper.h"
#include <format>
#include "Entity.h"


const bool Transform::registered = []() {
    ComponentFactory::registerType(ComponentType::Transform, []() { return std::make_unique<Transform>(); });
    return true;
    }();


void Transform::update()
{
    if (_isDirty) {
        _localModelMatrix = getLocalMatrix();
        _isDirty = false;
    }
    _globalModelMatrix = parent ? parent->_globalModelMatrix * _localModelMatrix : _localModelMatrix;

    for (auto&& child : children)
        child->update();
}

glm::mat4 Transform::getLocalMatrix()
{
    // Quaternion'u doğrudan rotasyon matrisine çeviriyoruz
    glm::mat4 rotationMatrix = glm::mat4_cast(_orientation);

    // translation * rotation * scale (also know as TRS matrix)
    return glm::translate(glm::mat4(1.0f), _position) *
        rotationMatrix *
        glm::scale(glm::mat4(1.0f), _scale);

}

void Transform::decomposeMatrix(const glm::mat4& matrix)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(matrix, scale, rotation, translation, skew, perspective);

	_position = translation;
    _orientation = rotation;
    _eulerRotation = glm::degrees(glm::eulerAngles(rotation));
    _scale = scale;
}

// it will works parents -> children
// bu if we call child before parent it will not work properly 
glm::mat4 Transform::getGlobalMatrix() {
    return _isDirty ? update(), _globalModelMatrix : _globalModelMatrix;
}


glm::vec3 Transform::getGlobalPosition() { 

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(_globalModelMatrix, scale, rotation, translation, skew, perspective);
	return translation;
}

glm::vec3 Transform::getPosition() { return _position; }
glm::vec3 Transform::getRotation() { return _eulerRotation; }
glm::quat Transform::getRotationAsQuat() { return _orientation; }
glm::vec3 Transform::getScale() { return _scale; }
void Transform::setPosition(const glm::vec3& position) { _position = position; _isDirty = true; }
void Transform::setRotation(const glm::vec3& eulerDegrees) {
    _eulerRotation = eulerDegrees;
    // Euler açılarını (derece) radyana çevirip Quaternion'a aktarıyoruz
    _orientation = glm::quat(glm::radians(eulerDegrees));
    _isDirty = true;
}
void Transform::setOrientation(const glm::quat& orientation){ _orientation = orientation; _isDirty = true; }
void Transform::setScale(const glm::vec3& scale) { _scale = scale; _isDirty = true; }

void Transform::move(const glm::vec3 &deltaPosition)
{
    setPosition(getPosition()+deltaPosition);
}

/// <summary>
/// Set local matrix of the transform according to world matrix
/// </summary>
/// <param name="worldMatrix"> Global matrix </param>
void Transform::setLocalMatrix(const glm::mat4& worldMatrix)
{
    glm::mat4 invParent(1);
    if(parent)
        invParent = glm::inverse(parent->getGlobalMatrix());

    glm::mat4 newLocal = invParent * worldMatrix;
	_localModelMatrix = newLocal;


    decomposeMatrix(newLocal);
    update();
}



void Transform::addChild(Transform* child)
{
    child->parent = this;
    children.push_back(child);
}
void Transform::removeChild(Transform* child)
{
	auto it = std::find(children.begin(), children.end(), child);
    children.erase(it, it+1);
    child->parent = nullptr;
}

void Transform::setParent(Transform* newParent)
{
    // parent bir nesnenin child bir nesneye atanma durumu engellenmeli
	// yani bir entity kendi child'ına child olamaz
	// bu sebeple is in children kontrolü yapılmalı
    // bool isAChild(newParent); 

	glm::mat4 worldMatrix = getGlobalMatrix();

    if (parent) 
        parent->removeChild(this);
    
    parent = newParent;
    if(parent)
	    parent->addChild(this);

    setLocalMatrix(worldMatrix);
}

void Transform::onInspect()
{
    bool p{false}, r{false}, s{false}, pr{false}, rr{false}, sr{false};
    ImGui::SeparatorText("Transform");
    
    if (ImGui::SmallButton("R##pos")){ _position = glm::vec3(0,0,0); pr = true; } ImGui::SameLine();
    p = ImGui::DragFloat3("Position:", &_position[0], 0.01);

    if (ImGui::SmallButton("R##rot")) { _eulerRotation = glm::vec3(0,0,0); rr = true; } ImGui::SameLine();
    r = ImGui::DragFloat3("Rotation:", &_eulerRotation[0], 1.00);

    if (ImGui::SmallButton("R##scale")) { _scale = glm::vec3(1,1,1); sr = true; } ImGui::SameLine();
    s = ImGui::DragFloat3("Scale:", &_scale[0], 0.01);
    
    if ( p || r || s || pr || rr || sr ) _isDirty = true;

    if(r || rr)
        _orientation = glm::quat(glm::radians(_eulerRotation));


    //Light* light = owner->getComponent<Light>();
    //if (p && light)
    //    light->position = _position;

    //if (r && light) {
    //    SpotLight* spotlight = dynamic_cast<SpotLight*>(light);
    //    if (spotlight)
    //        spotlight->setDirection(_rotation);
    //}

    //owner->onInspect();
}

void Transform::serialize(YAML::Emitter& out)
{
	out << YAML::BeginMap;
    Component::serialize(out);
	out << YAML::Key << "position" << YAML::Value << _position;
	out << YAML::Key << "orientation" << YAML::Value << _orientation;
	out << YAML::Key << "rotation" << YAML::Value << _eulerRotation;
	out << YAML::Key << "scale" << YAML::Value << _scale;

    uint64_t parentID = parent ? parent->UUID : 0;
    out << YAML::Key << "parentUUID" << YAML::Value << parentID;

    out << YAML::EndMap;

}

void Transform::deserialize(const YAML::Node& node)
{
	Component::deserialize(node);
    setPosition(node["position"].as<glm::vec3>());
    setRotation(node["rotation"].as<glm::vec3>());
    setOrientation(node["orientation"].as<glm::quat>());
    setScale(node["scale"].as<glm::vec3>());
}


//
//std::unique_ptr<Entity> TransformFactory::create(const YAML::Node& node, MaterialManager* materialManager)
//{
//	auto entity = std::make_unique<Entity>();
//    //auto entity1 = std::unique_ptr<Entity>();
//
//    entity->transform->name = node["name"].as<std::string>();
//    entity->transform->setPosition(node["position"].as<glm::vec3>());
//    entity->transform->setRotation(node["rotation"].as<glm::vec3>());
//    entity->transform->setScale(node["scale"].as<glm::vec3>());
//
//    //if (node["light"].IsDefined()) {
//    //    for (const auto& light : node["light"])
//    //        transform->getEntity()->light = std::move(LightFactory::create(light));
//    //    transform->getEntity()->light->position = transform->getPosition();
//    //}
//
//    //if (node["modelPath"].IsDefined()) {
//    //    std::string path = node["modelPath"].as<std::string>();
//    //    Model* model = new Model(materialManager);
//    //    model->loadFromFile(path);
//    //    transform->getEntity()->model.reset(model);
//    //    LOG_TRACE(std::format("Path:{}", path));
//
//    //}
//
//
//
//    //if (auto lightNode = node["light"]; lightNode && lightNode.IsDefined()) {
//    //    LOG_INFO("Light was found");
//    //}
//    //else {
//    //    LOG_ERROR("Light was not found");
//    //}
//
//    return entity;
//}
