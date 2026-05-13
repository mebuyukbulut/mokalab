#include "Renderer.h"

#include <glad/gl.h>
#include "Model.h"
#include "Shader.h"
#include "FileUtils.h"
#include "Camera.h"
#include "Logger.h"
#include "Texture.h"
#include <filesystem>
#include "AssetManager.h"
#include "RenderComponent.h"
#include "Transform.h"
#include "LightManager.h"
#include "Builtin.h"
#include "FX.h"
#include <execution>

void Renderer::initMatcap() {
    //matcapTexture = TextureFactory::load("data/matcaps/basic_1.png", false);
	
    // Scan directory for matcap textures    
    std::string path = "../assets/env/matcaps/";
    for (const auto& entry : std::filesystem::directory_iterator(path))
        matcapTexturePaths.push_back(entry.path().string());

    for(const auto& p : matcapTexturePaths)
		LOG_TRACE("Found matcap texture: {}", p);

    //matcapTexture = TextureFactory::load(matcapTexturePaths.front(), false);
    matcapTexture = g_Assets.get<Texture>( matcapTexturePaths.front());
}


void Renderer::shadowPass(const SceneRenderData &renderData) 
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(renderData.lightItems.size() == 0) return; 

    for(const LightItem& l :renderData.lightItems){
        Light* light = l.light;
        if(light->type != ComponentType::DirectionalLight) return;

        if(light->type == ComponentType::DirectionalLight){

            glm::mat4 lightProjection = light->getProjection();
            glm::mat4 lightView = light->getView();

            glm::mat4 lightSpaceMatrix = lightProjection * lightView; 

            _materialShader->use(); _materialShader->set("lightSpaceMatrix", lightSpaceMatrix);
            _shadowShader->use();
            _shadowShader->set("lightSpaceMatrix", lightSpaceMatrix);
            //_shadowShader->set("lightProjection", lightProjection);
            //_shadowShader->set("lightView", lightView);
            //_shadowShader->set("near_plane", near_plane);
            //_shadowShader->set("far_plane", far_plane);
            for (const RenderItem& item : renderData.renderItems) {
                drawModelWithShader(item.model, item.transform, _shadowShader);
            }
        }
    }
    
    

}

void Renderer::materialPass(const std::vector<RenderItem> &renderItems)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _materialShader->use();

    _shadowMapTarget.depthBuffer().bind(Builtin::TextureSlot::DirectionalShadowMap);
    // glActiveTexture(GL_TEXTURE0+Builtin::TextureSlot::DirectionalShadowMap);
    // glBindTexture(GL_TEXTURE_2D, _shadowMapTarget.depthBuffer());
    _materialShader->use();
    _materialShader->set("shadowMap", Builtin::TextureSlot::DirectionalShadowMap);

    for (const RenderItem& item : renderItems) 
        drawModelWithShader(item.model, item.transform, _materialShader);
}

void Renderer::matcapPass(const std::vector<RenderItem>& renderItems)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    
    matcapTexture->bind(0);
    _matcapShader->set("matcapTexture", 0);
    _matcapShader->use();

    for (const RenderItem& item : renderItems) 
        drawModelWithShader(item.model, item.transform, _matcapShader, false);
}

void Renderer::wireframePass(const std::vector<RenderItem>& renderItems)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    glLineWidth(1.f);

    _wireframeShader->use();
    for (const RenderItem& item : renderItems) 
        drawModelWithShader(item.model, item.transform, _wireframeShader);

    glEnable(GL_CULL_FACE);
}

void Renderer::backgroundPass()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);
    _backgroundShader->use();
    _bgModel->draw(_backgroundShader);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::gridPass()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    _gridShader->use();
    _gridModel->draw(_gridShader);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Renderer::lightPass(const std::vector<LightItem> &lightItems)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _lightShader->use();

    for (const LightItem & item : lightItems) {
        glm::mat4 newTransform;
        glm::vec3 newScale(0.1); 
        newTransform[0] = glm::normalize(item.transform[0]) * newScale.x;
        newTransform[1] = glm::normalize(item.transform[1]) * newScale.y;
        newTransform[2] = glm::normalize(item.transform[2]) * newScale.z;
        newTransform[3] = item.transform[3];

        _lightShader->set("lightColor", item.light->getGPULight().getColor());
        

        if(item.light->type == ComponentType::DirectionalLight)
            drawModelWithShader(_directionLightGizmo, newTransform, _lightShader);
        else if(item.light->type == ComponentType::PointLight)
            drawModelWithShader(_pointLightGizmo, newTransform, _lightShader);
        else if(item.light->type == ComponentType::SpotLight)
            drawModelWithShader(_spotLightGizmo, newTransform, _lightShader);
        else
            LOG_ERROR("Renderer::lightPass() -> UNKNOW LIGHT TYPE");
    }
        
}

void Renderer::selectionPass(const SceneRenderData &renderData)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glClearColor(0, 0, 0, 1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Selection pass for Models
	for (const RenderItem& item : renderData.renderItems) 
        drawModelWithShader(item.model, item.transform, _selectionShader, false, item.entityIndex + 1);

    // Selection pass for Lights
	for (const LightItem& item : renderData.lightItems){
        glm::mat4 newTransform;
        glm::vec3 newScale(0.1); 
        newTransform[0] = glm::normalize(item.transform[0]) * newScale.x;
        newTransform[1] = glm::normalize(item.transform[1]) * newScale.y;
        newTransform[2] = glm::normalize(item.transform[2]) * newScale.z;
        newTransform[3] = item.transform[3];

        if(item.light->type == ComponentType::DirectionalLight)
            drawModelWithShader(_directionLightGizmo, newTransform, _selectionShader, false, item.entityIndex + 1);
        else if(item.light->type == ComponentType::PointLight)
            drawModelWithShader(_pointLightGizmo, newTransform, _selectionShader, false, item.entityIndex + 1);
        else if(item.light->type == ComponentType::SpotLight)
            drawModelWithShader(_spotLightGizmo, newTransform, _selectionShader, false, item.entityIndex + 1);
        else
            LOG_ERROR("Renderer::lightPass() -> UNKNOW LIGHT TYPE");
    }
}

void Renderer::outlinePass(const SceneRenderData &renderData)
{
    // https://www.youtube.com/watch?v=bzEkWtY0zIY
    // Nuclear's graphics tricks #1: simple outline rendering    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(5.f);
    glCullFace(GL_FRONT);
    _wireframeShader->use();
    for (const RenderItem& item : renderData.renderItems) 
        if(item.isSelected)
            drawModelWithShader(item.model, item.transform, _wireframeShader);


    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    _materialShader->use();
    for (const RenderItem& item : renderData.renderItems) 
        if(item.isSelected)
            drawModelWithShader(item.model, item.transform, _materialShader, false); // ???



}

void Renderer::postProcessPass(const ColorRenderTarget& sourceTarget, ColorRenderTarget& destinationTarget, Shader* shader)
{
    destinationTarget.bind();
    clearBuffer();
    shader->use();
    shader->set("frameTex", 0); 
    sourceTarget.colorTexture().bind(0); 
    drawModelWithShader(_bgModel, glm::mat4(1.0), shader, false); 

    destinationTarget.unbind();
}

void Renderer::init(std::shared_ptr<Camera> camera) {
    _frameUniforms.init(); 

    // Biz bu değişkenleri hardcoded yazıyoruz ama bunları dosyadan okumak yada klasör taraması yapmak daha mantıklı olabilir
    // Ayrıca CWD path olayını da daha net bir hale getirmemiz gerekecek

    // Init all engine::shaders 
    std::vector<ShaderSettings> shaders{
        {Builtin::Shader::PBR, 		    "../assets/shaders/PBR0.vert", 			"../assets/shaders/PBR0.frag"},
        {Builtin::Shader::Matcap, 	    "../assets/shaders/matcap.vert", 		"../assets/shaders/matcap.frag"},
        {Builtin::Shader::Wireframe, 	"../assets/shaders/wireframe.vert", 	"../assets/shaders/wireframe.frag"},
        {Builtin::Shader::Background,   "../assets/shaders/bg_grad.vert", 		"../assets/shaders/bg_grad.frag"},
        {Builtin::Shader::Grid, 		"../assets/shaders/gridShader.vert", 	"../assets/shaders/gridShader.frag"},
        {Builtin::Shader::Selection, 	"../assets/shaders/selection.vert", 	"../assets/shaders/selection.frag"},
        {Builtin::Shader::Shadow,    	"../assets/shaders/simpleShadow.vert", 	"../assets/shaders/simpleShadow.frag"},
        {Builtin::Shader::Light,    	"../assets/shaders/light.vert", 	    "../assets/shaders/light.frag"},
    };
    //shaders.push_back({"lambertian", 	"../assets/shaders/lambertian.vert", 		"../assets/shaders/lambertian.frag"});
 	//shaders.push_back({"normal", 		"../assets/shaders/normal.vert", 			"../assets/shaders/normal.frag"});
 	//shaders.push_back({"particle0", 	"../assets/shaders/particle_point.vert", 	"../assets/shaders/particle_point.frag"});
 	//shaders.push_back({"blinn-phong",   "../assets/shaders/blinn-phong.vert", 		"../assets/shaders/blinn-phong.frag"});
 	//shaders.push_back({"basic", 		"../assets/shaders/basic_lighting.vert", 	"../assets/shaders/basic_lighting.frag"});
 	//shaders.push_back({"skybox", 		"../assets/shaders/skybox.vert", 			"../assets/shaders/skybox.frag"});
 	//shaders.push_back({"hdr2cubemap",   "../assets/shaders/hdr2cubemap.vert", 		"../assets/shaders/hdr2cubemap.frag"});
 	
    for(auto& ss : shaders)
        g_Assets.get<Shader>(ss.name, &ss);
    
    
	// Setup rendering passes
    _shadowShader       = g_Assets.get<Shader>(Builtin::Shader::Shadow).get();
    _materialShader     = g_Assets.get<Shader>(Builtin::Shader::PBR).get();
	_matcapShader       = g_Assets.get<Shader>(Builtin::Shader::Matcap).get();
    _wireframeShader    = g_Assets.get<Shader>(Builtin::Shader::Wireframe).get();
	_backgroundShader   = g_Assets.get<Shader>(Builtin::Shader::Background).get();
	_gridShader         = g_Assets.get<Shader>(Builtin::Shader::Grid).get();
    _lightShader        = g_Assets.get<Shader>(Builtin::Shader::Light).get();
	_selectionShader    = g_Assets.get<Shader>(Builtin::Shader::Selection).get();
    


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glEnable(GL_BLEND);

    

    //// Initialize skybox
    //std::vector<std::string> faces
    //{
    //    "data/skybox/right.jpg",
    //    "data/skybox/left.jpg",
    //    "data/skybox/top.jpg",
    //    "data/skybox/bottom.jpg",
    //    "data/skybox/front.jpg",
    //    "data/skybox/back.jpg"
    //};
    //cubemapTexture = TextureFactory::loadCubeMap(faces);
    ////cubemapTexture = TextureFactory::loadHDR("data/HDRs/citrus_orchard_road_puresky_1k.hdr", &_shaderManager.getShader("hdr2cubemap"));

    // Mesh tmp = MeshFactory::create(DefaultShapes::Cube);
    // _bgMesh = new Mesh(tmp);
    


    _bgModel    = g_Assets.get<Model>(Builtin::Model::BgPlane).get();
    _gridModel  = g_Assets.get<Model>(Builtin::Model::GridPlane).get();

	_directionLightGizmo    = g_Assets.get<Model>(Builtin::Model::Cone).get();
	_pointLightGizmo        = g_Assets.get<Model>(Builtin::Model::Cube).get();
	_spotLightGizmo         = g_Assets.get<Model>(Builtin::Model::Cone).get();


    //glDisable(GL_FRAMEBUFFER_SRGB);
    //GLboolean srgbEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    //std::cout << "Framebuffer sRGB: " << (srgbEnabled ? "ENABLED" : "DISABLED") << std::endl;

	initMatcap();

    setCamera(camera);

    setViewMode(ViewMode::Material);


    _rt.create(300,300); // create default frame buffer for viewport
    _postProcA.create(300,300); // create default frame buffer for viewport
    _postProcB.create(300,300);
    _shadowMapTarget.create(2048,2048);
    _st.create(300,300);

}
void Renderer::terminate() {
    //_shaderManager.terminate(); 
}

void Renderer::renderScene(const SceneRenderData &renderData, bool isViewportSelect)
{
    _shadowMapTarget.bind();
    clearBuffer();
    shadowPass(renderData);
    _shadowMapTarget.unbind();


    _frameUniforms.update(_camera->getViewMatrix(), _camera->getProjectionMatrix(), _camera->getPosition());
    


    if(isViewportSelect){
        _st.bind();
        selectionPass(renderData);
    }

    
    
    _rt.bind();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);    
    clearBuffer();


    backgroundPass();
    if      (_viewMode == ViewMode::Material)   materialPass(renderData.renderItems);
    else if (_viewMode == ViewMode::Matcap)     matcapPass(renderData.renderItems);
    else if (_viewMode == ViewMode::Wireframe)  wireframePass(renderData.renderItems);
    
    // World-space Overlay Effects
    lightPass(renderData.lightItems);
    gridPass();

    outlinePass(renderData); 

    _rt.unbind();


    // Post Processing START
    glDisable(GL_DEPTH_TEST);

    const std::vector<FXInstance>& postProcessStack = fxReg->getActiveFXStack();
    if(postProcessStack.size() == 0) {
        _finalTarget = &_rt; 
    }
    else if(postProcessStack.size() == 1) {
        postProcessPass(_rt, _postProcA, postProcessStack[0].getShader());
        _finalTarget = &_postProcA; 
    }
    else{
        postProcessPass(_rt, _postProcA, postProcessStack[0].getShader());
        int i ;
        for(i = 1; i < postProcessStack.size(); i++){
            if(i%2)
                postProcessPass(_postProcA, _postProcB, postProcessStack[i].getShader());
            else
                postProcessPass(_postProcB, _postProcA, postProcessStack[i].getShader());      
        }

        if(i%2)
            _finalTarget = &_postProcA; 
        else
            _finalTarget = &_postProcB; 

    }
    // postProcessPass(_rt, _postProcA, g_Assets.get<Shader>(Builtin::FX::Grayscale).get());
    // postProcessPass(_postProcA, _postProcB, g_Assets.get<Shader>(Builtin::FX::Pixelate).get());

    

    glEnable(GL_DEPTH_TEST);
    // Post Processing END

}

void Renderer::clearBuffer(){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void Renderer::resizeViewport(int width, int height)
{
    bool isResized = _rt.resize(width, height); 
    _postProcA.resize(width,height) ; 
    _postProcB.resize(width,height); 
    _st.resize(width,height); 

    if(isResized)
        _camera->setWindowSize(width,height);
}

GLuint Renderer::getViewportImage()
{
    return _finalTarget->colorTexture().getId(); 
}

GLuint Renderer::getDebugImage()
{ 
    return _shadowMapTarget.depthBuffer().getId(); 
}

void Renderer::drawModelWithShader(Model* model, const glm::mat4& transform, Shader* shader, bool bindMaterial, uint32_t ID){

    shader->use();
    shader->set("model", transform);
    if(ID)
        shader->set("objectID", ID);

    model->draw(shader, bindMaterial);
}

// only return non-zero values. 
std::vector<uint32_t> Renderer::getSelections(glm::vec2 mousePosBegin, glm::vec2 mousePosEnd)
{
    uint32_t x = glm::min(mousePosBegin.x, mousePosEnd.x);
    uint32_t y = glm::min(mousePosBegin.y, mousePosEnd.y);

    glm::vec2 delta = glm::abs(mousePosEnd-mousePosBegin) + glm::vec2(1,1);
    uint32_t deltaX = delta.x;
    uint32_t deltaY = delta.y; 
    // LOG_CRITICAL("{}, {}", deltaX, deltaY);

    std::vector<uint32_t> pickedIDs;   
    pickedIDs.resize(deltaX*deltaY);
    

    
    _st.bind();
    glReadPixels(x, y, deltaX, deltaY, GL_RED_INTEGER, GL_UNSIGNED_INT, pickedIDs.data());
    _st.unbind();

    if (pickedIDs.size() > 100000) { // Sadece 100 bin elemandan fazlaysa paralel yap
        std::sort(std::execution::par, pickedIDs.begin(), pickedIDs.end());
        auto last = std::unique(std::execution::par, pickedIDs.begin(), pickedIDs.end());
        pickedIDs.erase(last, pickedIDs.end());
    } else {
        std::sort(pickedIDs.begin(), pickedIDs.end());
        auto last = std::unique(pickedIDs.begin(), pickedIDs.end());
        pickedIDs.erase(last, pickedIDs.end());
    }

    // 5. Sıfırı (arkaplan/boşluk) temizle
    pickedIDs.erase(std::remove(pickedIDs.begin(), pickedIDs.end(), 0), pickedIDs.end());
    
    return pickedIDs;
}

void Renderer::setViewMode(ViewMode mode) {_viewMode = mode; }
ViewMode Renderer::getViewMode()          {return _viewMode; }

void Renderer::setCamera(std::shared_ptr<Camera> camera) { _camera = camera; }



ColorRenderTarget::ColorRenderTarget()
{
    colorTex = new Texture();
    depthTex = new Texture();
}

ColorRenderTarget::~ColorRenderTarget()
{
    delete colorTex;
    delete depthTex;
}

void ColorRenderTarget::create(int width, int height)
{
    this->width = width; this->height = height; 

    colorTex->createColorTexture(width, height);
    depthTex->createDepthTexture(width, height);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex->getId(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->getId(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("ColorRenderTarget incomplete!");

    unbind(); // bu burada gereksiz gibi ama bakalım
}

void ColorRenderTarget::destroy()
{    
    glDeleteFramebuffers(1, &fbo);
    colorTex->destroy();
    depthTex->destroy();
}

bool ColorRenderTarget::resize(int width, int height)
{    
    if (width == this->width && height == this->height // if(newSize == oldSize) -> do not create new framebuffer
        || (!width || !height) )// or if(newSize.x == 0 || newSize.y == 0) -> do not create new framebuffer
        return false; 

    // Eski GPU kaynaklarını serbest bırak
    destroy();

    // Yeni boyutla tekrar oluştur
    create(width, height);
    return true; 
}

void ColorRenderTarget::bind(){
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void ColorRenderTarget::unbind(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ColorRenderTarget::framebuffer()  const { return fbo; }
Texture& ColorRenderTarget::colorTexture() const { return *colorTex; }
Texture& ColorRenderTarget::depthBuffer()  const { return *depthTex; }

ShadowMapTarget::ShadowMapTarget()
{
    depthMap = new Texture();
}

ShadowMapTarget::~ShadowMapTarget()
{
    delete depthMap; 
}

//
void ShadowMapTarget::create(int width, int height)
{
    this->width = width; this->height = height;

    depthMap->createShadowDepthTexture(width, height);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap->getId(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    unbind();
}

void ShadowMapTarget::destroy()
{
    glDeleteFramebuffers(1, &fbo);
    depthMap->destroy();
}

// width height !width !height !h&&!w expected !h||!w
// 0       0       1     1       1       1       1
// 0       1       1     0       0       1       1
// 1       0       0     1       0       1       1
// 1       1       0     0       0       0       0

bool ShadowMapTarget::resize(int width, int height)
{
    if (width == this->width && height == this->height // if(newSize == oldSize) -> do not create new framebuffer
        || (!width || !height) )// or if(newSize.x == 0 || newSize.y == 0) -> do not create new framebuffer
        return false; 

    // Eski GPU kaynaklarını serbest bırak
    destroy();

    // Yeni boyutla tekrar oluştur
    create(width, height);
    return true; 
}

void ShadowMapTarget::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void ShadowMapTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

GLuint ShadowMapTarget::framebuffer() const { return fbo; }
Texture& ShadowMapTarget::depthBuffer() const { return *depthMap; }



SelectionRenderTarget::SelectionRenderTarget()
{
    idTex = new Texture();
    depthTex = new Texture();
}
SelectionRenderTarget::~SelectionRenderTarget()
{
    // neden fbo yu destroy etmedik? Etmemiz lazım!
    delete idTex;
    delete depthTex;
}

void SelectionRenderTarget::create(int width, int height)
{
    this->width = width; this->height = height; 

    idTex->createIdTexture(width, height);
    depthTex->createDepthTexture(width, height);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTex->getId(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->getId(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("SelectionRenderTarget incomplete!");

    unbind();
}

void SelectionRenderTarget::destroy()
{
    glDeleteFramebuffers(1, &fbo);
    idTex->destroy();
    depthTex->destroy();
}

bool SelectionRenderTarget::resize(int width, int height)
{    
    if (width == this->width && height == this->height // if(newSize == oldSize) -> do not create new framebuffer
        || (!width || !height) )// or if(newSize.x == 0 || newSize.y == 0) -> do not create new framebuffer
        return false; 

    // Eski GPU kaynaklarını serbest bırak
    destroy();

    // Yeni boyutla tekrar oluştur
    create(width, height);
    return true; 
}

void SelectionRenderTarget::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}
void SelectionRenderTarget::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint SelectionRenderTarget::framebuffer()   const { return fbo; }
Texture &SelectionRenderTarget::idTexture()   const { return *idTex; }
Texture &SelectionRenderTarget::depthBuffer() const { return *depthTex; }
