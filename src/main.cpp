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
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
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

struct Star {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    GLenum drawMode;
    float offsetX, offsetY;

    Star(const std::vector<float>& vertices, GLenum mode, float x=0.0f, float y=0.0f)
        : drawMode(mode), offsetX(x), offsetY(y)
    {
        vertexCount = vertices.size() / 3;

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

struct Ray {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    GLenum drawMode;
    float offsetX, offsetY;
    float localMinX, localMaxX;
    float localMinY, localMaxY;

    Ray(const std::vector<float>& vertices, GLenum mode, float x=0.0f, float y=0.0f)
        : drawMode(mode), offsetX(x), offsetY(y),
          localMinX(0.0f), localMaxX(0.0f),
          localMinY(0.0f), localMaxY(0.0f)
    {
        vertexCount = vertices.size() / 3;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Compute local X and Y extents from vertex data
        for (int i = 0; i < (int)vertices.size(); i += 3) {
            float vx = vertices[i];
            float vy = vertices[i + 1];
            if (i == 0) {
                localMinX = localMaxX = vx;
                localMinY = localMaxY = vy;
            } else {
                if (vx < localMinX) localMinX = vx;
                if (vx > localMaxX) localMaxX = vx;
                if (vy < localMinY) localMinY = vy;
                if (vy > localMaxY) localMaxY = vy;
            }
        }
    }

    void draw(unsigned int shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, offsetX, offsetY);

        glDrawArrays(drawMode, 0, vertexCount);
    }

    void updateLoc(float xMove = 0.0f, float yMove = 0.0f) {

        this->localMaxX += xMove;
        this->localMaxY += yMove;

        // Update the right endpoint (x2, y2) in the VBO directly (So we can refresh the screen and still keep the streak)
        // Fixes diagonal line being really thick at its start
        // Second vertex starts at byte offset 12 (3 floats * 4 bytes)
        float newX = this->localMaxX;
        float newY = this->localMaxY;


        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 12, sizeof(float), &newX); // update x2
        glBufferSubData(GL_ARRAY_BUFFER, 16, sizeof(float), &newY); // update y2
    }

    bool collidedWith(const Star& star, float radius = 0.3f) {
        float aspect = 800.0f / 600.0f;

        // Right tip of the line in world space
        float tipX = this->localMaxX + this->offsetX;

        // Y position of the line in world space
        float tipY = this->localMaxY + this->offsetY;

        // Check distance from center of star (account for aspect squishing)
        float dx = (star.offsetX - tipX) * aspect;
        float dy = star.offsetY - tipY;

        float dist = sqrt(dx*dx + dy*dy);
        //It cannot be closer than 0.3 (since that is the radius of the star)
        return dist <= radius;
    }
};

// Helper function: create circle vertices
std::vector<float> createCircle(float radius, int segments) {
    std::vector<float> verts;
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

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
    }

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
    Ray line3(createLine(-0.9f, -0.5f, -0.85f, -0.5f), GL_LINES);
    Ray line4(createLine(0.75f, -0.9f, 0.75f, -0.85f), GL_LINES);
    Ray line5(createLine(-1.0f, -1.0f, -0.95f, -0.95f), GL_LINES);


    while(!glfwWindowShouldClose(window)){

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        circle.draw(starShaderProgram);
        line.draw(lineShaderProgram);
        line2.draw(lineShaderProgram);
        line3.draw(lineShaderProgram);
        line4.draw(lineShaderProgram);
        line5.draw(lineShaderProgram);

        if (!line.collidedWith(circle)) {
            line.updateLoc(0.002, 0);
        }
        if (!line2.collidedWith(circle)) {
            line2.updateLoc(0.002, 0);
        }
        if (!line3.collidedWith(circle)) {
            line3.updateLoc(0.002, 0);
        }
        if (!line4.collidedWith(circle)) {
            line4.updateLoc(0, 0.002);
        }
        if (!line5.collidedWith(circle)) {
            line5.updateLoc(0.002, 0.001);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}