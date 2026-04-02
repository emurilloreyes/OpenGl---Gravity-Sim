#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define M_PI 3.141592

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec2 offset; // for moving objects
void main()
{
    gl_Position = vec4(aPos.xy + offset, aPos.z, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // blueish
}
)";

const char* fragmentShaderSourceStar = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 1.0f, 0.0f, 0.0f);
}
)";

struct Ray {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    GLenum drawMode;
    float offsetX, offsetY;

    Ray(const std::vector<float>& vertices, GLenum mode, float x=0.0f, float y=0.0f)
        : drawMode(mode), offsetX(x), offsetY(y)
    {
        vertexCount = vertices.size() / 3; // since it stores x, y, and z coordinates

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(unsigned int shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, offsetX, offsetY);

        glDrawArrays(drawMode, 0, vertexCount);
    }

    void updateLoc() {
        this->offsetX += .001;
    }
};

struct Star {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    GLenum drawMode;
    float offsetX, offsetY;

    Star(const std::vector<float>& vertices, GLenum mode, float x=0.0f, float y=0.0f)
        : drawMode(mode), offsetX(x), offsetY(y)
    {
        vertexCount = vertices.size() / 3; // since it stores x, y, and z coordinates

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(unsigned int shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, offsetX, offsetY);

        glDrawArrays(drawMode, 0, vertexCount);
    }
};

// General Shape struct
// struct Shape {
//     unsigned int VAO;
//     unsigned int VBO;
//     int vertexCount;
//     GLenum drawMode;
//     float offsetX, offsetY;

//     Shape(const std::vector<float>& vertices, GLenum mode, float x=0.0f, float y=0.0f)
//         : drawMode(mode), offsetX(x), offsetY(y)
//     {
//         vertexCount = vertices.size() / 3; // since it stores x, y, and z coordinates

//         glGenVertexArrays(1, &VAO);
//         glGenBuffers(1, &VBO);

//         glBindVertexArray(VAO);
//         glBindBuffer(GL_ARRAY_BUFFER, VBO);
//         glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//         glEnableVertexAttribArray(0);
//     }

//     void draw(unsigned int shaderProgram) {
//         glUseProgram(shaderProgram);
//         glBindVertexArray(VAO);

//         int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
//         glUniform2f(offsetLoc, offsetX, offsetY);

//         glDrawArrays(drawMode, 0, vertexCount);
//     }

//     void updateLoc() {
//         this->offsetX += .001;
//     }

//     bool checkCollision(const Shape& other, float radius = 0.3f) {
//         float aspect = 800.0f / 600.0f;
    
//         // Right tip of the line in world space
//         float lineTipX = this->offsetX + (-0.85f);
        
//         // Left edge of the circle in world space (radius is squeezed by aspect)
//         float circleEdge = other.offsetX - (radius / aspect);
        
//         return lineTipX >= circleLeftEdge;

//     }
// };

// Helper function: create circle vertices
std::vector<float> createCircle(float radius, int segments) {
    std::vector<float> verts;
    // center
    verts.push_back(0.0f);
    verts.push_back(0.0f);
    verts.push_back(0.0f);
    float aspect = 800.0f / 600.0f;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        verts.push_back(radius * cos(angle) / aspect);
        verts.push_back(radius * sin(angle));
        verts.push_back(0.0f);
    }
    return verts;
}

// Helper function: create a line
std::vector<float> createLine(float x1, float y1, float x2, float y2) {
    return { x1, y1, 0.0f, x2, y2, 0.0f };
}

// Compile shader and create program
unsigned int createShaderProgram(const char* &fragmentShaderSource) {
    int success;
    char infoLog[512];

    // Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
    }

    // Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
    }

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Program Error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Initialize GLFW
    if(!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Shapes", nullptr, nullptr);
    if(!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }

    glViewport(0, 0, 800, 600);

    unsigned int starShaderProgram = createShaderProgram(fragmentShaderSourceStar);
    unsigned int lineShaderProgram = createShaderProgram(fragmentShaderSource);

    // Create shapes
    Star circle(createCircle(0.3f, 100), GL_TRIANGLE_FAN, 0.75f, 0.0f);
    Ray line(createLine(-0.9f, 0.0f, -0.85f, 0.0f), GL_LINES);
    Ray line2(createLine(-0.9f, -0.2f, -0.85f, -0.2f), GL_LINES);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    while(!glfwWindowShouldClose(window)){

        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

        // Draw all shapes
        circle.draw(starShaderProgram);
        line.draw(lineShaderProgram);
        line2.draw(lineShaderProgram);
        //if (!line.checkCollision(circle)) {
            line.updateLoc();
        //}
        //if (!line2.checkCollision(circle)) {
            line2.updateLoc();
        //}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}