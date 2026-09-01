#version 410 core

layout (location = 0) in vec3 aPosition;

out vec3 tex;

layout(std140) uniform skyboxBuffer
{
    mat4 proj;
    mat4 view;
};

void main() {
    gl_Position = proj * view * vec4(aPosition, 1.0f);
    tex = aPosition;
}
