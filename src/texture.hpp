#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

#include "loader.hpp"
#include "shader.hpp"

#include <array>
#include <glad/glad.h>

static constexpr uint8_t cubeFaces = 6;

constexpr std::array<const char*, cubeFaces> skyboxTextures
    = {ASSETS_DIR "/cubemap_posx.png", ASSETS_DIR "/cubemap_negx.png",
       ASSETS_DIR "/cubemap_posy.png", ASSETS_DIR "/cubemap_negy.png",
       ASSETS_DIR "/cubemap_posz.png", ASSETS_DIR "/cubemap_negz.png"};

class Texture {
 public:
    const char* m_uniform;
    GLuint      m_id;
    GLuint      m_texid;
    GLenum      m_type;

    Texture(const char* uniform, GLuint texid, GLenum type)
        : m_texid(texid),
          m_uniform(uniform),
          m_type(type) {
        glGenTextures(1, &m_id);
    }

    void set_texid(const Shader shader);
    void bind();
    void unbind();
    void destroy();
};

class Texture2D : public Texture {
 public:
    int m_width, m_height;
    Texture2D(int width, int height, const char* uniform, GLuint texid);

    void resize(int width, int height);
};

class CubeMap : public Texture {
 public:
    CubeMap(std::array<const char*, cubeFaces> files,
            const char*                        uniform,
            GLuint                             texid);
};

#endif
