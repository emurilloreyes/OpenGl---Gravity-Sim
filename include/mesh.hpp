#pragma once

#include <vector>
#include <glad/glad.h>
#include "math.hpp"

const char* defaultVertexShader();
const char* defaultFragmentShaderWhite();
const char* defaultFragmentShaderRed();

unsigned int createShaderProgram(const char* vertexSrc, const char* fragmentSrc);

std::vector<float> createSphere(float radius, int stacks, int slices);
std::vector<float> createLineStrip(const Vec3& start, const Vec3& dir, float length, int segments);

class Mesh {
public:
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    int vertexCount = 0;
    GLenum drawMode = GL_TRIANGLES;

    Mesh() = default;
    Mesh(const std::vector<float>& vertices, GLenum mode, GLenum usage = GL_STATIC_DRAW);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw(unsigned int shaderProgram, const Mat4& mvp) const;
};
