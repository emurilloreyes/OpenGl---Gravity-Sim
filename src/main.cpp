#include <iostream>
#include <vector>
#include <cmath>
#include <glad/glad.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "constants.hpp"
#include "geodesic.hpp"
#include "mesh.hpp"

struct InputState {
    OrbitCamera* camera = nullptr;
    bool dragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

struct PhotonVisual {
    Photon photon;
    Mesh mesh;

    PhotonVisual(double b, double approachAngle, double planeTiltX)
        : mesh(makeDynamicLineStrip()) {
        photon.resetFromDirection(b, approachAngle, planeTiltX);
        mesh.updateVertices(photon.path);
    }

    void step() {
        if (!photon.isActive()) return;
        for (int i = 0; i < constants::geodesicStepsPerFrame && photon.isActive(); ++i) {
            photon.step(constants::dPhi, constants::rs, constants::rMax);
        }
        mesh.updateVertices(photon.path);
    }

    void draw(unsigned int shader, const Mat4& mvp) const {
        if (photon.path.size() >= 6) {
            mesh.draw(shader, mvp);
        }
    }
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

    Mesh blackHole(createSphere(constants::bhRenderRadius, 24, 36), GL_TRIANGLES);

    // Schwarzschild geodesics: many directions (azimuth), impact parameters b, tilted orbital planes.
    std::vector<PhotonVisual> photons;
    photons.reserve(static_cast<size_t>(constants::rayDirectionCount) * 16 *
                    static_cast<size_t>(constants::rayTiltCount));

    for (int i = 0; i < constants::rayDirectionCount; ++i) {
        const double approachAngle =
            (2.0 * M_PI * i) / static_cast<double>(constants::rayDirectionCount);

        for (double b = constants::rayBMin; b <= constants::rayBMax + 1e-9; b += constants::rayBStep) {
            for (int t = 0; t < constants::rayTiltCount; ++t) {
                double tilt = 0.0;
                if (constants::rayTiltCount > 1) {
                    tilt = -constants::rayTiltMax +
                           (2.0 * constants::rayTiltMax * t) /
                               static_cast<double>(constants::rayTiltCount - 1);
                }
                photons.emplace_back(b, approachAngle, tilt);
            }
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 vp = camera.viewProjection();
        Mat4 model = Mat4::identity();
        Mat4 mvp = multiply(vp, model);

        for (auto& p : photons) {
            p.step();
        }

        blackHole.draw(bhShader, mvp);
        for (const auto& p : photons) {
            p.draw(rayShader, mvp);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
