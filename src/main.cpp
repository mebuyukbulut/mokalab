#define STB_IMAGE_IMPLEMENTATION
#include "SplashScreen.h"
#include "Engine.h"

// #include <windows.h>
// extern "C"
// {
// 	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
// 	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
// }

void init(){
	// glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	// glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	// glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	// glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	// glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
}
void terminate(){

    glfwTerminate();
}
int main()
{
	init();

	// SPLASH SCREEN
	SplashScreen splash;
	auto path = splash.run();

	if(path.empty()){
		terminate();
		return EXIT_SUCCESS;
	}

	// ENGINE
	Engine engine;
	engine.run();

	terminate();

	return EXIT_SUCCESS;
}
// ╔══════════════════════════════════════════════════════════════════════════╗
// ║	NAMING CONVENTION				                                      ║
// ╚══════════════════════════════════════════════════════════════════════════╝    
// ┌──────────────────────────────────────────────────────────────────────────┐
// │ Element			┆ Style				┆ Example						  │
// │┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄│
// │ Class				┆ PascalCase		┆ ModelViewer, ShaderProgram	  │
// │ Function			┆ camelCase()		┆ drawModel(), loadShader()		  │
// │ Member Variable	┆ _camelCase		┆ _camera, _pbrShader			  │
// │ Parameter			┆ camelCase			┆ setCamera(const Camera& camera) │
// │ Local Variable		┆ camelCase			┆ modelMatrix, shaderProgram	  │
// │ Constants			┆ UPPER_CASE		┆ PI, 							  │
// │ Enums				┆ PascalCase		┆ RenderMode::Wireframe			  │
// └──────────────────────────────────────────────────────────────────────────┘
// 
// 