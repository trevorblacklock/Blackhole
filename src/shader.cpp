#include "shader.hpp"

std::string get_file_contents(std::string filename) {
    std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
    if (!file.is_open())
        throw std::runtime_error("Failed to open " + filename);
    using Iterator = std::istreambuf_iterator<char>;
    std::string content(Iterator {file}, Iterator {});
    if (!file)
        throw std::runtime_error("Failed to read " + filename);
    return content;
}

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    // Read shader files
    std::string vertexCode   = get_file_contents(vertexFile);
    std::string fragCode     = get_file_contents(fragmentFile);
    const char* vertexSource = vertexCode.c_str();
    const char* fragSource   = fragCode.c_str();


    // Create the vertex shader object
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    compile_errors(vertexShader, "VERTEX");

    // Create fragment shader object
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    compile_errors(fragShader, "FRAGMENT");

    // Create shader program object
    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragShader);
    glLinkProgram(m_id);
    compile_errors(m_id, "PROGRAM");

    // Delete the shaders now that they have been compiled
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
}

void Shader::activate() {
    glUseProgram(m_id);
}

void Shader::deactivate() {
    glUseProgram(0);
}

void Shader::destroy() {
    glDeleteProgram(m_id);
}

void Shader::compile_errors(uint32_t shader, const char* type) {
    GLint hasCompiled;
    char  infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (!hasCompiled) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << '\n'
                      << infoLog << std::endl;
        }

    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (!hasCompiled) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << '\n'
                      << infoLog << std::endl;
        }
    }
}
