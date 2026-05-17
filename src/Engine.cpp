#include "Engine.h"
#include "Shader.h"
#include "FileUtils.h"
#include "Model.h"
#include "Renderer.h"
#include "Material.h"
#include "Texture.h"
#include <functional>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include "Mouse.h"

#include "EventDispatcher.h"
#include "Logger.h"
#include "AssetManager.h"


void Engine::initWindow()
{   // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    // --------------------
    _window = glfwCreateWindow(
        config.window.width, 
        config.window.height, 
        config.window.title.c_str(),
        config.window.fullscreen ? glfwGetPrimaryMonitor() : NULL,
        NULL);
    if (_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        //return -1;
    }
    glfwMakeContextCurrent(_window);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, Engine::framebuffer_size_callback);
    glfwSetDropCallback(_window, [](GLFWwindow* window, int count, const char** paths) {
        for (int i = 0; i < count; ++i) {
            Event e{ EventType::ModelOpened, {} };
            e.data = std::make_unique<EventData_Text>(std::string(paths[i]));
            dispatcher.dispatch(e);
        }
        });

    _mouse.init(_window, &_UI);
    dispatcher.subscribe(EventType::onMove, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Point> p(static_cast<EventData_Point*>(e.release()));
        _camera->move(p->vec);
        });
    dispatcher.subscribe(EventType::onRotate, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Point> p(static_cast<EventData_Point*>(e.release()));
        _camera->rotate(p->vec);
        });
    dispatcher.subscribe(EventType::onZoom, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Point> p(static_cast<EventData_Point*>(e.release()));
        _camera->zoom(p->vec.y);
        });
    dispatcher.subscribe(EventType::SetMainWindowTitle, [&](std::unique_ptr<EventData> e) {
        std::unique_ptr<EventData_Text> t(static_cast<EventData_Text*>(e.release()));
        glfwSetWindowTitle(_window, t->text.c_str());
		});
}

void Engine::initOpenGL()
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0) {
        LOG_ERROR("Failed to initialize OpenGL context\n");
		//return -1;
	}
    
    if (!GL_ARB_bindless_texture) {
        LOG_ERROR("Bindless textures is not supported! (GL_ARB_bindless_texture)");
        // Geriye dönük (fallback) çözüm: Texture Array veya standart slot yönetimi
        exit(EXIT_FAILURE);
    }else{
        LOG_SUCCESS("Bindless textures is supported! (GL_ARB_bindless_texture)");
    }

}

void Engine::initUI()
{
    _UI.init(_window, _camera);
	//_UI.setWindowSize(SCR_WIDTH, SCR_HEIGHT);
    
    dispatcher.subscribe(EventType::ShaderSelected, [&](std::unique_ptr<EventData> e) {
        //_renderer.setShader(e.data.text,Renderer::ShaderType::Main);
        });
    dispatcher.subscribe(EventType::EngineExit, [&](std::unique_ptr<EventData> e) {
        SM.saveScene("");
        glfwSetWindowShouldClose(_window, true);
        });


    _UI.ps = &ps;
}


void Engine::init(){
    config.load();

    initWindow();
	initOpenGL();    
    
	_camera->init(glm::vec2(config.window.width, config.window.height));
	_camera->setWindowSize(config.window.width, config.window.height);

    _renderer.init(_camera);

    SM.init(&_renderer, _camera.get(), &_UI);

	initUI();


    LOG_SUCCESS("Engine was initialized");
}
void Engine::mainLoop()
{
    ///_renderer.setShader("particle0");
    /////ParticleSystem ps;
    ///ps.init(_camera);
    
    time.init();
    // render loop
    while (!glfwWindowShouldClose(_window))
    {
        //processInput(_window);
        _IM.processInput(_window);
        //_renderer.beginFrame();

        time.update();
        double deltaTime = time.deltaTime();
        _mouse.update(deltaTime);
        //ps.update(deltaTime);
        //ps.draw();
        
        SM.draw();
        g_Assets.update(); 
        
        
		_UI.draw(&SM);


        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}
void Engine::terminate() {
    _renderer.terminate();
    _UI.terminate();
    glfwTerminate();
}

void Engine::run()
{
    init();
    mainLoop();
	terminate();
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);

    // Retrieve the instance pointer
    Engine* app = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (app) {
        LOG_TRACE("Window resized to: {} x {}",width,height);
        app->_renderer.resizeViewport(width, height);
    }
}