#pragma once
#include "Model.h"
#include "Entity.h"
#include "RenderItem.h"
#include <memory>

class Camera;
class Shader;
class Mesh; 
class Model;
class Texture; 

enum class ViewMode {
	Wireframe,
	Matcap,
	Material,
};


class FrameUniforms{
    struct GPUFrameUniforms {
        glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 viewPos; float padding; // 16 byte'a tamamlamak için
    }blockData;

	GLuint uboID; // Uniform Buffer Object ID
public:
	void init(){
		glGenBuffers(1, &uboID);
		glBindBuffer(GL_UNIFORM_BUFFER, uboID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUFrameUniforms), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboID); // 0 numaralı binding point
	}
	~FrameUniforms(){
		glDeleteBuffers(1, &uboID);
	}

	void update(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos){
		blockData.view = view;
		blockData.projection = projection;
		blockData.viewPos = viewPos; 

		glBindBuffer(GL_UNIFORM_BUFFER, uboID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUFrameUniforms), &blockData);
	}

};




// A render buffer that can hold color and depth values 
// For know we use it for render scene in 3D viewport 
// and selection 
class ColorRenderTarget {
	GLuint fbo;
	Texture* colorTex{};
	Texture* depthTex{};
	int width, height;

public:
	ColorRenderTarget();
	~ColorRenderTarget();
	void create(int width, int height);
	void destroy();
	bool resize(int width, int height);
	void bind();
	void unbind();

	// Getters
	GLuint framebuffer() const;
    Texture& colorTexture() const;
    Texture& depthBuffer() const;
};



// 
class ShadowMapTarget {
	GLuint fbo;
	Texture* depthMap{};
	int width, height;

public:
	ShadowMapTarget();
	~ShadowMapTarget();
	void create(int width, int height);
	void destroy();
	bool resize(int width, int height);
	void bind();
	void unbind();

	// Getters
	GLuint framebuffer() const;
    Texture& depthBuffer() const;
};



class SelectionRenderTarget {
	GLuint fbo;
	Texture* idTex{};
	Texture* depthTex{};
	int width, height;

public:
	SelectionRenderTarget();
	~SelectionRenderTarget();
	void create(int width, int height);
	void destroy();
	bool resize(int width, int height);
	void bind();
	void unbind();

	// Getters
	GLuint framebuffer() const;
    Texture& idTexture() const;
    Texture& depthBuffer() const;
};




class Renderer
{
public:

	enum class ShaderType{
		Main,
		Background,
		Grid,
		Light,
		Selection,
		Matcap,
		Material, 
	};
private:
	ColorRenderTarget _rt{};
	ColorRenderTarget _postProcA{}, _postProcB{};
	ColorRenderTarget* _finalTarget{};
	ShadowMapTarget _shadowMapTarget{};
	SelectionRenderTarget _st{};
	FrameUniforms _frameUniforms{};

	ViewMode _viewMode{ ViewMode::Material };

	Shader* _shadowShader{};
	Shader* _materialShader{};
	Shader* _matcapShader{};
	Shader* _wireframeShader{};
	Shader* _backgroundShader{};
	Shader* _gridShader{};
	Shader* _lightShader{};
	Shader* _selectionShader{};

	std::shared_ptr<Camera> _camera;
	uint32_t lastSelectedID{};

    Model* _bgModel{};
    Model* _gridModel{};

	Model* _directionLightGizmo{};
	Model* _pointLightGizmo{};
	Model* _spotLightGizmo{};

	Texture* cubemapTexture;
	//std::vector<Texture*> loadedMatcapTextures; 
	std::shared_ptr<Texture> matcapTexture;
	std::vector<std::string> matcapTexturePaths;
	void initMatcap();
	//void initSkybox();

	void shadowPass(const SceneRenderData &renderData);
	void materialPass(const std::vector<RenderItem>& renderItems);
	void matcapPass(const std::vector<RenderItem>& renderItems);
	void wireframePass(const std::vector<RenderItem>& renderItems);
	void backgroundPass();
	void gridPass();
	void lightPass(const std::vector<LightItem>& lightItems);
	void selectionPass(const SceneRenderData &renderData);
	void outlinePass(const SceneRenderData &renderData);
	void postProcessPass(const ColorRenderTarget& sourceTarget, ColorRenderTarget& destinationTarget, Shader* shader);

	void drawModelWithShader(Model* model, const glm::mat4& transform, Shader* shader, bool bindMaterial = true, uint32_t ID = 0);

public:
	class FXRegistry* fxReg{};
	
	void init(std::shared_ptr<Camera> camera);
	void terminate();

	void renderScene(const SceneRenderData &renderData, bool isViewportSelect, glm::vec2 mousePos);
	void clearBuffer();

	void resizeViewport(int width, int height);
	GLuint getViewportImage();
	GLuint getDebugImage();
	

	uint32_t getSelection(glm::vec2 mousePos);

	void setViewMode(ViewMode mode);
	ViewMode getViewMode();

	void setCamera(std::shared_ptr<Camera> camera);

};