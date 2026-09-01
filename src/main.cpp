#include "loader.hpp"
#include "objects.hpp"
#include "render.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>


int main(int argc, char* argv[]) {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int width  = 800;
    const int height = 800;

    auto window = create_window(width, height, "Blackhole");
    if (!window) {
        std::cerr << "Failed to create an OpenGL window" << std::endl;
        glfwTerminate();
        return 1;
    }

    if (!gladLoadGL()) {
        std::cerr << "Failed to load OpenGL functions" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    auto vertdir = SHADER_DIR "/simple.vert";
    auto fragdir = SHADER_DIR "/kerr.frag";
    
    Shader  skyboxShader(vertdir, fragdir);
    CubeMap skyboxTexture(skyboxTextures, "skybox", 0);
    Cube    skyboxCube(1000);
    Object  skybox(skyboxTexture, skyboxCube, skyboxShader);
    UniformBuffer<CameraData>    skyboxBuffer(0, "skyboxBuffer", skyboxShader);
    UniformBuffer<BlackholeData> blackholeBuffer(1, "blackholeBuffer", skyboxShader);
        
    RenderItem items(skybox); 
        

    Render render(window);
    render.fill_buffers(skyboxBuffer, blackholeBuffer);
    render.loop(items);

    skyboxShader.destroy();
    skyboxTexture.destroy();
    glfwTerminate();
}
