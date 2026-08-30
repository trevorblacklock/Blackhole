#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "objects.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <GLFW/glfw3.h>
#include <atomic>
#include <future>
#include <glad/glad.h>
#include <thread>

struct alignas(16) CameraData {
    glm::mat4 proj;
    glm::mat4 view;
};

struct alignas(16) BlackholeData {
    glm::vec4 pos;
    glm::vec4 camPos;
    double    time;

    BlackholeData()
        : pos(glm::vec4(0.0f)),
          camPos(glm::vec4(0.0f)),
          time(0.0f) {
    }
};

struct FdBuffer {
    // Keep an iterator to loop through each set of data to update
    std::vector<void*> m_iterator;
    // Store the size to error check
    size_t m_size;

    // Unique constructor to control data iterator, this assumes
    // each buffer will be updated with a contiguous block of memory
    // corresponding to each data structure.
    template<typename T, typename... Ts>
    FdBuffer(T* first, Ts*... following) : m_size(1 + sizeof...(Ts)) {
        m_iterator = {first, following...};
    }
};

struct RenderItem {
    Object skybox;
};

class Render {
 private:
    // Store the window width and height
    bool  m_firstClick;
    int   m_width, m_height;
    float m_fov, m_near, m_far;

    // Current window context
    GLFWwindow* m_window;

    // Store the camera orientation and position
    glm::vec3 m_orientation;
    glm::vec3 m_position;
    glm::vec3 m_up;
    float     m_radius;

    // Store certain types of data
    CameraData    m_cameraData;
    BlackholeData m_blackholeData;

    // Store the uniform buffers
    std::vector<BaseUniformBuffer> m_buffers;

    std::atomic<bool> m_resize;

    static void framebuffer_callback(GLFWwindow* window, int width, int height);
    void        handle_inputs();
    void        event_loop();
    // Update the uniform buffers
    void update_ubuffers(FdBuffer fdbuffer);
    // Update parameters in the camera each frame
    void update_viewport();

 public:
    // Constructor for the renderer
    Render(GLFWwindow* window);

    // Render loop
    void loop(RenderItem item);

    // Fill uniform buffers
    template<std::derived_from<BaseUniformBuffer>... Ts>
    void fill_buffers(Ts... buffers) {
        m_buffers = {buffers...};
    }
};

// Helper to create and return a GLFWwindow, makes this window the current
// context
inline GLFWwindow* create_window(int width, int height, const char* title) {
    auto window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return window;
}

#endif
