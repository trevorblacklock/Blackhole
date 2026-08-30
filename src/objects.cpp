#include "objects.hpp"

CubeLayers::CubeLayers(uint32_t scale)
    : m_scale(scale),
      m_indices(cubeIndices) {
    // Scale the default vertex
    for (auto i = 0; i < m_vertices.size(); ++i)
        m_vertices[i] = scale * cubeVertices[i];
}

void BaseUniformBuffer::init() {
    // Create a ubo
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    glBufferData(GL_UNIFORM_BUFFER, m_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // Create a binding point for the shader
    glBindBufferBase(GL_UNIFORM_BUFFER, m_binding, m_id);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void BaseUniformBuffer::bind_to_shader(const Shader shader) {
    // Find the uniform index associated with the given shader
    GLuint idx = glGetUniformBlockIndex(shader.m_id, m_uniform);
    // Check if index is valid
    if (idx == GL_INVALID_INDEX) {
        std::cerr << "Could not find block uniform index." << std::endl;
        exit(-1);
    }
    // Bind the shader to the given uniform
    glUniformBlockBinding(shader.m_id, idx, m_binding);
}

void BaseUniformBuffer::update(void* data, const uint32_t offset) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, m_size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

RenderBufferDS::RenderBufferDS(int width, int height)
    : RenderBuffer(width, height) {
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width,
                          m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderBufferDS::resize(int width, int height) {
    // Reset the width and height
    m_width  = width;
    m_height = height;
    // Resize the render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width,
                          m_height);
}

FrameBuffer2D::FrameBuffer2D(Texture2D texture)
    : FrameBuffer(),
      m_texture(texture),
      m_rbo(RenderBufferDS(texture.m_width, texture.m_height)) {
    init();
}

void FrameBuffer2D::init() {
    // Bind the framebuffer, renderbuffer and colorbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glBindTexture(GL_TEXTURE_2D, m_texture.m_id);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo.m_id);

    // Attach the render buffer to depth and stencil attachment of frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_rbo.m_id);
    // Set the texture color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_texture.m_id, 0);
    // Ensure frame buffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Warning, incomplete framebuffer object." << std::endl;
    // Unbind the framebuffer, renderbuffer and colorbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Vertex::Vertex(const float*    vertices,
               size_t          vertSize,
               const uint32_t* indices,
               size_t          idxSize,
               uint32_t        layout,
               uint32_t        dim,
               void*           offset) {
    // Store the number of vertices and indices there are
    m_numVertices = vertSize / sizeof(float);
    m_numIndices  = idxSize / sizeof(uint32_t);

    // Create the vao, vbo, ebo and ubo
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    // Bind the vao
    glBindVertexArray(m_vao);

    // Bind and buffer the vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

    // Bind and buffer the ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize, indices, GL_STATIC_DRAW);

    // Link the vao and vbo
    glVertexAttribPointer(layout, dim, GL_FLOAT, false, dim * sizeof(float),
                          offset);
    glEnableVertexAttribArray(layout);

    // Unbind the vao, vbo and ebo
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Object::Object(Texture texture, Vertex vertex, Shader shader)
    : m_texture(texture),
      m_vertex(vertex),
      m_shader(shader) {
    // Activate the shader and set the texture id
    m_shader.activate();
    m_texture.set_texid(m_shader);
    m_shader.deactivate();
}

void Object::draw() {
    m_shader.activate();
    // Bind the vao
    glBindVertexArray(m_vertex.m_vao);
    // Bind the texture and set active
    glActiveTexture(GL_TEXTURE0);
    m_texture.bind();
    // Draw the vertices
    glDrawElements(GL_TRIANGLES, m_vertex.m_numIndices, GL_UNSIGNED_INT, 0);
    // Unbind the vao and texture
    glBindVertexArray(0);
    m_texture.unbind();
}
