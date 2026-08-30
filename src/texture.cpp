#include "texture.hpp"

void Texture::set_texid(const Shader shader) {
    // Make sure a uniform exists
    if (!m_uniform)
        return;
    // Retrieve the associated shader index for the uniform
    GLint idx = glGetUniformLocation(shader.m_id, m_uniform);
    if (idx == -1) {
        std::cerr << "Invalid shader uniform: " << m_uniform << std::endl;
        // exit(-1);
    }
    glUniform1i(idx, m_texid);
}

void Texture::bind() {
    glBindTexture(m_type, m_id);
}

void Texture::unbind() {
    glBindTexture(m_type, 0);
}

void Texture::destroy() {
    glDeleteTextures(1, &m_id);
}

Texture2D::Texture2D(int width, int height, const char* uniform, GLuint texid)
    : Texture(uniform, texid, GL_TEXTURE_2D),
      m_width(width),
      m_height(height) {
    // Bind the texture
    glBindTexture(m_type, m_id);
    // Define the image type
    glTexImage2D(m_type, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    // Set the filters
    glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Clamp textures to edge
    glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Unbind texture when everything is done
    glBindTexture(m_type, 0);
}

void Texture2D::resize(int width, int height) {
    // Set new width and height
    m_width  = width;
    m_height = height;
    // Bind the texture and resize it
    glBindTexture(m_type, m_id);
    glTexImage2D(m_type, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    // Set the filters
    glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Clamp textures to edge
    glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Unbind texture when everything is done
    glBindTexture(m_type, 0);
}

CubeMap::CubeMap(std::array<const char*, cubeFaces> files,
                 const char*                        uniform,
                 GLuint                             texid)
    : Texture(uniform, texid, GL_TEXTURE_CUBE_MAP) {
    // Bind the texture
    glBindTexture(m_type, m_id);
    // Set the filters
    glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Clamp textures to edge
    glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Load the images here
    ImageLoader loader(files);
    // Loop through each face
    for (auto i = 0; i < loader.m_images.size(); ++i) {
        const auto& image = loader.m_images[i];
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                     image.m_width, image.m_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     image.m_data.data());
    }
    // Unbind texture now that everything is done
    glBindTexture(m_type, 0);
}
