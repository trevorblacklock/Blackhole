#ifndef OBJECTS_HPP_INCLUDED
#define OBJECTS_HPP_INCLUDED

#define GLM_ENABLE_EXPERIMENTAL

#include "shader.hpp"
#include "texture.hpp"

#include <array>
#include <concepts>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <valarray>

// Vertices for a cube of lengths 1x1x1.
constexpr std::array<float, 24> cubeVertices
    = {-1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f, -1.0f,
       -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f,
       1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f, -1.0f};

// Index order for cube
constexpr std::array<uint32_t, 36> cubeIndices
    = {1, 2, 6, 6, 5, 1, 0, 4, 7, 7, 3, 0, 4, 5, 6, 6, 7, 4,
       0, 3, 2, 2, 1, 0, 0, 1, 5, 5, 4, 0, 3, 7, 6, 6, 2, 3};

// Vertices for a quad that covers the screen
constexpr std::array<float, 8> quad2DVertices
    = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};

// Indices for a quad that covers the screen
constexpr std::array<uint32_t, 6> quad2DIndices = {0, 1, 2, 0, 3, 2};

struct CubeLayers {
    std::array<float, 24>    m_vertices;
    std::array<uint32_t, 36> m_indices;
    uint32_t                 m_scale;

    CubeLayers(uint32_t scale);
};

class BaseUniformBuffer {
 protected:
    GLuint      m_id;
    GLuint      m_binding;
    size_t      m_size;
    const char* m_uniform;

    void init();

 public:
    template<std::same_as<Shader>... Ts>
    BaseUniformBuffer(GLuint      binding,
                      size_t      size,
                      const char* uniform,
                      Ts... shaders)
        : m_binding(binding),
          m_size(size),
          m_uniform(uniform) {
        // Initialize and allocate a buffer
        init();
        // Bind each shader now
        for (const auto& shader : {shaders...})
            bind_to_shader(shader);
    }

    void bind_to_shader(const Shader shader);
    void update(void* data, const uint32_t offset = 0);
};

template<typename T>
class UniformBuffer : public BaseUniformBuffer {
 public:
    template<std::same_as<Shader>... Ts>
    UniformBuffer(GLuint binding, const char* uniform, Ts... shaders)
        : BaseUniformBuffer(binding, sizeof(T), uniform, shaders...) {
    }
};

class RenderBuffer {
 protected:
    int m_width, m_height;

 public:
    GLuint m_id;

    RenderBuffer(int width, int height) : m_width(width), m_height(height) {
        glGenRenderbuffers(1, &m_id);
    }

    void bind() {
        glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    }

    void unbind() {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
};

class RenderBufferDS : public RenderBuffer {
 public:
    RenderBufferDS(int width, int height);

    void resize(int width, int height);
};

class FrameBuffer {
 protected:
    GLuint m_id;

 public:
    FrameBuffer() {
        glGenFramebuffers(1, &m_id);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class FrameBuffer2D : public FrameBuffer {
 private:
    Texture2D      m_texture;
    RenderBufferDS m_rbo;

 public:
    FrameBuffer2D(Texture2D texture);

    void init();

    void resize(int width, int height) {
        // Resize texture without deleting it
        m_texture.resize(width, height);
        // Resize the rbo without deleting it
        m_rbo.resize(width, height);
        // Re attach everything to the render buffer
        init();
    }
};

class Vertex {
 public:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    uint32_t m_numVertices;
    uint32_t m_numIndices;

    Vertex(const float*    vertices,
           size_t          vertSize,
           const uint32_t* indices,
           size_t          idxSize,
           uint32_t        layout,
           uint32_t        dim,
           void*           offset);
};

class Cube : public CubeLayers, public Vertex {
 public:
    Cube(uint32_t size)
        : CubeLayers(size),
          Vertex(m_vertices.data(),
                 sizeof(m_vertices),
                 m_indices.data(),
                 sizeof(m_indices),
                 0,
                 3,
                 0) {
    }
};

class Quad2D : public Vertex {
 public:
    Quad2D()
        : Vertex(quad2DVertices.data(),
                 sizeof(quad2DVertices),
                 quad2DIndices.data(),
                 sizeof(quad2DIndices),
                 0,
                 2,
                 0) {
    }
};

class Object {
 public:
    // Store basic attributes of the object
    Texture m_texture;
    Vertex  m_vertex;
    Shader  m_shader;

    // Base constructor for an "object"
    Object(Texture texture, Vertex vertex, Shader shader);

    // Draw the object each frame
    void draw();
};

#endif
