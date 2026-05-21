#include "mesh.hpp"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const char* defaultVertexShader() {
    return R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";
}

const char* defaultFragmentShaderWhite() {
    return R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
}

const char* defaultFragmentShaderRed() {
    return R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";
}

unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    int success = 0;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static void pushVertex(std::vector<float>& verts, float radius, float phi, float theta) {
    float y = radius * std::cos(phi);
    float ring = radius * std::sin(phi);
    verts.push_back(ring * std::cos(theta));
    verts.push_back(y);
    verts.push_back(ring * std::sin(theta));
}

std::vector<float> createSphere(float radius, int stacks, int slices) {
    std::vector<float> verts;
    verts.reserve(static_cast<size_t>(stacks) * slices * 6 * 3);

    for (int i = 0; i < stacks; ++i) {
        float phi0 = static_cast<float>(M_PI) * i / stacks;
        float phi1 = static_cast<float>(M_PI) * (i + 1) / stacks;

        for (int j = 0; j < slices; ++j) {
            float t0 = 2.0f * static_cast<float>(M_PI) * j / slices;
            float t1 = 2.0f * static_cast<float>(M_PI) * (j + 1) / slices;

            pushVertex(verts, radius, phi0, t0);
            pushVertex(verts, radius, phi1, t0);
            pushVertex(verts, radius, phi1, t1);

            pushVertex(verts, radius, phi0, t0);
            pushVertex(verts, radius, phi1, t1);
            pushVertex(verts, radius, phi0, t1);
        }
    }
    return verts;
}

std::vector<float> createLineStrip(const Vec3& start, const Vec3& dir, float length, int segments) {
    std::vector<float> verts;
    Vec3 d = dir.normalized();
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments * length;
        Vec3 p = start + d * t;
        verts.push_back(p.x);
        verts.push_back(p.y);
        verts.push_back(p.z);
    }
    return verts;
}

Mesh::Mesh(const std::vector<float>& vertices, GLenum mode, GLenum usage) : drawMode(mode) {
    vertexCount = static_cast<int>(vertices.size() / 3);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), usage);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

Mesh::Mesh(Mesh&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), vertexCount(other.vertexCount), drawMode(other.drawMode) {
    other.VAO = other.VBO = 0;
    other.vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        VAO = other.VAO;
        VBO = other.VBO;
        vertexCount = other.vertexCount;
        drawMode = other.drawMode;
        other.VAO = other.VBO = 0;
        other.vertexCount = 0;
    }
    return *this;
}

void Mesh::draw(unsigned int shaderProgram, const Mat4& mvp) const {
    glUseProgram(shaderProgram);
    int loc = glGetUniformLocation(shaderProgram, "uMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, mvp.m);

    glBindVertexArray(VAO);
    glDrawArrays(drawMode, 0, vertexCount);
    glBindVertexArray(0);
}
