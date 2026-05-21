#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "mesh.hpp"

struct InputState {
    OrbitCamera* camera = nullptr;
    bool dragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (input && input->camera && height > 0) {
        input->camera->setAspect(static_cast<float>(width) / static_cast<float>(height));
    }
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || !input->camera || !input->dragging) return;

    float dx = static_cast<float>(xpos - input->lastX);
    float dy = static_cast<float>(ypos - input->lastY);
    input->lastX = xpos;
    input->lastY = ypos;

    input->camera->orbit(dx * 0.005f, -dy * 0.005f);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (!input || button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (action == GLFW_PRESS) {
        input->dragging = true;
        glfwGetCursorPos(window, &input->lastX, &input->lastY);
    } else if (action == GLFW_RELEASE) {
        input->dragging = false;
    }
}

static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    auto* input = static_cast<InputState*>(glfwGetWindowUserPointer(window));
    if (input && input->camera) {
        input->camera->zoom(static_cast<float>(yoffset) * 0.8f);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: simulation.exe <width> <height>\n";
        return 1;
    }

    int windowWidth = static_cast<int>(std::stof(argv[1]));
    int windowHeight = static_cast<int>(std::stof(argv[2]));

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Black Hole (3D)", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    OrbitCamera camera;
    camera.setAspect(static_cast<float>(windowWidth) / static_cast<float>(windowHeight));

    InputState input;
    input.camera = &camera;
    glfwSetWindowUserPointer(window, &input);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    unsigned int bhShader = createShaderProgram(defaultVertexShader(), defaultFragmentShaderRed());
    unsigned int rayShader = createShaderProgram(defaultVertexShader(), defaultFragmentShaderWhite());

    const float bhRadius = 0.35f;
    Mesh blackHole(createSphere(bhRadius, 24, 36), GL_TRIANGLES);

    // Straight rays in 3D: grid on plane x = -6, direction +x (physics comes in Phase B).
    const float rayStartX = -6.0f;
    const float rayLength = 12.0f;
    const int raySegments = 32;

    std::vector<Mesh> rays;
    for (float y = -2.0f; y <= 2.0f; y += 0.4f) {
        for (float z = -2.0f; z <= 2.0f; z += 0.4f) {
            Vec3 start{rayStartX, y, z};
            Vec3 dir{1.0f, 0.0f, 0.0f};
            rays.emplace_back(createLineStrip(start, dir, rayLength, raySegments), GL_LINE_STRIP);
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 vp = camera.viewProjection();
        Mat4 model = Mat4::identity();

        blackHole.draw(bhShader, multiply(vp, model));
        for (const auto& ray : rays) {
            ray.draw(rayShader, multiply(vp, model));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
