#include "loader.hpp"
#include "objects.hpp"
#include "render.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int width  = 800;
    const int height = 800;

    auto window = create_window(width, height, "Blackhole");

    gladLoadGL();

    Shader  skyboxShader("shaders/simple.vert", "shaders/kerr.frag");
    CubeMap skyboxTexture(skyboxTextures, "skybox", 0);
    Cube    skyboxCube(1000);
    Object  skybox(skyboxTexture, skyboxCube, skyboxShader);
    UniformBuffer<CameraData>    skyboxBuffer(0, "skyboxBuffer", skyboxShader);
    UniformBuffer<BlackholeData> blackholeBuffer(1, "blackholeBuffer",
                                                 skyboxShader);

    RenderItem items(skybox);

    Render render(window);
    render.fill_buffers(skyboxBuffer, blackholeBuffer);
    render.loop(items);

    skyboxShader.destroy();
    skyboxTexture.destroy();
    glfwTerminate();
}
