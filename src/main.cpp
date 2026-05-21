#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define M_PI 3.141592

const float c = 299792458;
const float GConst = 6.67e-11;

const float kPhysicsDt = 0.01f;
const float kRaySpeed = 0.1f;

static void rotateToward(float& dirX, float& dirY, float targetX, float targetY, float deltaTheta) {
    float len = sqrtf(targetX * targetX + targetY * targetY);
    if (len < 1e-12f) return;
    targetX /= len;
    targetY /= len;

    float cross = dirX * targetY - dirY * targetX;
    float dot = dirX * targetX + dirY * targetY;
    float angle = atan2f(cross, dot);
    if (fabsf(angle) < 1e-12f) return;

    float rot = (angle > 0.0f ? 1.0f : -1.0f) * fminf(fabsf(angle), deltaTheta);
    float cosR = cosf(rot);
    float sinR = sinf(rot);
    float newDirX = dirX * cosR - dirY * sinR;
    float newDirY = dirX * sinR + dirY * cosR;
    dirX = newDirX;
    dirY = newDirY;
}

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec2 offset;
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

const char* fragmentShaderSourceBH = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

struct BlackHole {
    unsigned int VAO;
    unsigned int VBO;
    int vertexCount;
    GLenum drawMode;
    float offsetX, offsetY;
    float mass;
    float rs_ndc;

    BlackHole(const std::vector<float>& vertices, float mass, GLenum mode, float radius, float x=0.0f, float y=0.0f)
        : drawMode(mode), rs_ndc(radius), offsetX(x), offsetY(y), mass(mass)
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
    std::vector<float> path;
    float offsetX, offsetY;
    float aspect;
    float dirX, dirY;
    float currentX, currentY;

    Ray(float startX, float startY, float winWidth, float winHeight, float initDirX=1.0f, float initDirY=0.0f)
    : offsetX(0.0f), offsetY(0.0f),
      aspect(winWidth / winHeight),
      currentX(startX), currentY(startY)
    {
        float mag = sqrtf(initDirX * initDirX + initDirY * initDirY);
        dirX = initDirX / mag;
        dirY = initDirY / mag;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Store all of the rays in a vector 'remember them'
        // Helps solve the issue of just drawing a straight line between two points 
        // When orbiting the black hole their trajectory gets traced properly
        path.push_back(startX);
        path.push_back(startY);
        path.push_back(0.0f);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, path.size() * sizeof(float), path.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(unsigned int shaderProgram) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, offsetX, offsetY);

        glDrawArrays(GL_LINE_STRIP, 0, path.size() / 3);
    }

    void step(const BlackHole& bh, float mppX, float mppY, float dt, float speed) {
        float dxAspect = (bh.offsetX - currentX) * aspect;
        float dy = bh.offsetY - currentY;

        float dxMeters = dxAspect * mppX;
        float dyMeters = dy * mppY;
        float distMeters = sqrtf(dxMeters * dxMeters + dyMeters * dyMeters);
        if (distMeters < 1e-6f) return;

        // Weak-field deflection per arc length (rad/m): 4GM / (c^2 r^2)
        float kappa = 4.0f * GConst * bh.mass / (c * c * distMeters * distMeters);

        float stepMetersX = dirX * speed * dt * aspect * mppX;
        float stepMetersY = dirY * speed * dt * mppY;
        float dsMeters = sqrtf(stepMetersX * stepMetersX + stepMetersY * stepMetersY);
        float deltaTheta = kappa * dsMeters;

        float toBhX = bh.offsetX - currentX;
        float toBhY = bh.offsetY - currentY;
        rotateToward(dirX, dirY, toBhX, toBhY, deltaTheta);

        currentX += dirX * speed * dt;
        currentY += dirY * speed * dt;

        path.push_back(currentX);
        path.push_back(currentY);
        path.push_back(0.0f);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, path.size() * sizeof(float), path.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    bool collidedWith(const BlackHole& bh) {

        // Dist to center of black hole
        float dx = (bh.offsetX - currentX) * aspect;
        float dy = bh.offsetY - currentY;

        float dist = sqrt(dx*dx + dy*dy);

        // Just assume the rs is the radius for simplicity
        return dist <= bh.rs_ndc;
    }
};

// Helper function: create circle vertices
std::vector<float> createCircle(float radius, int segments, float winWidth, float winHeight) {
    std::vector<float> verts;
    verts.push_back(0.0f);
    verts.push_back(0.0f);
    verts.push_back(0.0f);
    float aspect = winWidth / winHeight;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        verts.push_back(radius * cos(angle) / aspect);
        verts.push_back(radius * sin(angle));
        verts.push_back(0.0f);
    }
    return verts;
}

// Compile shader and create program
unsigned int createShaderProgram(const char* &fragmentShaderSource) {
    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main(int argc, char *argv[]) {

    float windowWidth = std::stof(argv[1]);
    float windowHeight = std::stof(argv[2]);

    float metersPerPixelX = 1e8f / windowWidth;
    float metersPerPixelY = 1e8f / windowHeight;

    if(!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Black Hole", nullptr, nullptr);
    if(!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);

    unsigned int bhShader = createShaderProgram(fragmentShaderSourceBH);
    unsigned int lightShader = createShaderProgram(fragmentShaderSource);

    BlackHole black_hole(createCircle(0.05f, 100, windowWidth, windowHeight), 5e30f, GL_TRIANGLE_FAN, 0.05f, 0.5f, 0.0f);

    // Create a row of light rays down the right
    std::vector<Ray> rays;
    for (float y = -1.0f; y <= 1.0f; y += 0.05f)
        rays.emplace_back(-1.0f, y, windowWidth, windowHeight, 0.05f, 0.0f);

    while(!glfwWindowShouldClose(window)){

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        black_hole.draw(bhShader);
        // Check collisions and update position of each ray
        for (auto& r : rays) {
            r.draw(lightShader);
            if (!r.collidedWith(black_hole)) {
                r.step(black_hole, metersPerPixelX, metersPerPixelY, kPhysicsDt, kRaySpeed);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}