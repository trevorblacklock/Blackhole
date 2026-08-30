#ifndef SHADER_HPP_INCLUDED
#define SHADER_HPP_INCLUDED

#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

std::string get_file_contents(std::string filename);

class Shader {
 public:
    // Reference id of the shader program
    GLuint m_id;
    // Constructor that builds the shader program from a vertex and fragment
    // shader
    Shader(const char* vertexFile, const char* fragmentFile);
    // Activates the shader
    void activate();
    // Deactivate the shader
    void deactivate();
    // Deletes the shader
    void destroy();

 private:
    void compile_errors(uint32_t shader, const char* type);
};

#endif
