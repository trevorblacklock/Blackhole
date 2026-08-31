#include "render.hpp"

#include <numeric>

Render::Render(GLFWwindow* window)
    : m_fov(90.0f),
      m_near(0.1f),
      m_far(10000.0f),
      m_resize(false),
      m_window(window) {
    // Read the current width and height
    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    // Set a default position and orientation
    m_position    = glm::vec3(0.0f, 0.0f, -30.0f);
    m_orientation = glm::vec3(0.0f, 0.0f, 1.0f);
    m_up          = glm::vec3(0.0f, 1.0f, 0.0f);
    m_radius      = glm::length(m_position);
    // Set default projection and view matrices
    m_cameraData.proj = glm::perspective(
        glm::radians(m_fov), (float)m_width / (float)m_height, m_near, m_far);
    m_cameraData.view
        = glm::lookAt(m_position, m_position + m_orientation, m_up);

    // Set the camera position
    m_blackholeData.camPos = glm::vec4(m_position, 1.0f);

    // Setup the framebuffer size callback (for resizing the window)
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, &Render::framebuffer_callback);
    // Create the viewport
    glViewport(0, 0, m_width, m_height);
}

// Update the uniform buffers
void Render::update_ubuffers(FdBuffer fdbuffer) {
    // Ensure sizes are equal
    if (fdbuffer.m_size != m_buffers.size()) {
        std::cout << "Number of passed uniform buffers does not match iterator"
                  << std::endl;
        exit(-1);
    }
    // Iterate through each buffer and its respective data pointer
    for (auto i = 0; i < m_buffers.size(); ++i)
        m_buffers[i].update(fdbuffer.m_iterator[i]);
}

void Render::framebuffer_callback(GLFWwindow* window, int width, int height) {
    // Set the width and height
    Render* ptr   = static_cast<Render*>(glfwGetWindowUserPointer(window));
    ptr->m_width  = width;
    ptr->m_height = height;
    ptr->m_cameraData.proj = glm::perspective(glm::radians(ptr->m_fov),
                                              (float)width / (float)height,
                                              ptr->m_near, ptr->m_far);
    ptr->m_resize          = true;
}

void Render::update_viewport() {
    // Check if a resize is required
    if (m_resize) {
        glViewport(0, 0, m_width, m_height);
        m_resize = false;
    }
}

void Render::loop(RenderItem item) {

    // Create a framedata buffer
    FdBuffer fdbuffer(&m_cameraData, &m_blackholeData);

    float start      = static_cast<float>(glfwGetTime());
    float frameStart = start;

    std::vector<float> times;

    // Loop until the window closes
    while (!glfwWindowShouldClose(m_window)) {
        // Update the viewport
        frameStart = glfwGetTime();

        glfwPollEvents();
        handle_inputs();    

        update_viewport();
        // Clear the color and depth buffer bit
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        // Update the uniform buffers
        update_ubuffers(fdbuffer);
        // Draw the skybox
        item.skybox.draw();
        // Swap buffers
        glfwSwapBuffers(m_window);
        // Update time
        float time = static_cast<float>(glfwGetTime());
        times.push_back((time - frameStart));
        if (times.size() >= 20) {
            auto average = times.size()
                         / std::accumulate(times.begin(), times.end(), 0.0f);
            std::cout << '\r' << average << " fps" << std::flush;
            times.clear();
        }
        m_blackholeData.time = time - start;
    }
    // Wait for event thread to finish
    std::cout << std::endl;
}

void Render::handle_inputs() {
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        // Hides mouse cursor
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (m_firstClick) {
            glfwSetCursorPos(m_window, m_width / 2, m_height / 2);
            m_firstClick = false;
        }

        // Stores the coordinates of the cursor
        double mx;
        double my;
        // Fetches the coordinates of the cursor
        glfwGetCursorPos(m_window, &mx, &my);

        // Calculate mouse movement delta from center
        float dx = (float)(mx - m_width / 2);
        float dy = (float)(my - m_height / 2);

        // Sensitivity control
        const float sensitivity = 0.005f;

        // Calculate spherical coordinates
        glm::vec3 direction = m_position; // Since we're orbiting (0,0,0)
        float     radius    = glm::length(direction);

        // Convert to spherical angles
        float theta = acos(direction.y / radius);      // Polar angle
        float phi   = atan2(direction.z, direction.x); // Azimuthal angle

        // Update angles based on mouse movement
        phi -= dx * sensitivity;
        theta += dy * sensitivity;

        // Clamp theta to avoid flipping at poles
        theta = glm::clamp(theta, 0.1f, glm::pi<float>() - 0.1f);

        // Convert back to Cartesian coordinates
        m_position.x = radius * sin(theta) * cos(phi);
        m_position.y = radius * cos(theta);
        m_position.z = radius * sin(theta) * sin(phi);

        // Update orientation to look at (0,0,0)
        m_orientation = glm::normalize(-m_position);

        // Reset cursor to center
        glfwSetCursorPos(m_window, m_width / 2, m_height / 2);

        // Update view matrix
        m_cameraData.view      = glm::lookAt(m_position, glm::vec3(0.0f), m_up);
        m_blackholeData.camPos = glm::vec4(m_position, 1.0f);
    } else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)
               == GLFW_RELEASE) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_firstClick = true;
    }
}
