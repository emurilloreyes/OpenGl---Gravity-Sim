// Headless check: trajectory shape should not depend on dt or speed scaling.
#include <cmath>
#include <cstdio>
#include <vector>

const float c = 299792458.0f;
const float GConst = 6.67e-11f;

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

struct SimParams {
    float bhX, bhY, mass, aspect, mppX, mppY;
};

static std::vector<float> runPath(float dt, float speed, int steps, const SimParams& p) {
    float dirX = 1.0f, dirY = 0.0f;
    float x = -1.0f, y = 0.5f;
    std::vector<float> xs, ys;
    xs.push_back(x);
    ys.push_back(y);

    for (int i = 0; i < steps; ++i) {
        float dxAspect = (p.bhX - x) * p.aspect;
        float dy = p.bhY - y;
        float dxMeters = dxAspect * p.mppX;
        float dyMeters = dy * p.mppY;
        float distMeters = sqrtf(dxMeters * dxMeters + dyMeters * dyMeters);
        if (distMeters < 1e-6f) break;

        float kappa = 4.0f * GConst * p.mass / (c * c * distMeters * distMeters);
        float stepMetersX = dirX * speed * dt * p.aspect * p.mppX;
        float stepMetersY = dirY * speed * dt * p.mppY;
        float dsMeters = sqrtf(stepMetersX * stepMetersX + stepMetersY * stepMetersY);
        float deltaTheta = kappa * dsMeters;

        rotateToward(dirX, dirY, p.bhX - x, p.bhY - y, deltaTheta);
        x += dirX * speed * dt;
        y += dirY * speed * dt;
        xs.push_back(x);
        ys.push_back(y);
    }
    std::vector<float> flat;
    for (size_t i = 0; i < xs.size(); ++i) {
        flat.push_back(xs[i]);
        flat.push_back(ys[i]);
    }
    return flat;
}

static float maxDiffByArcLength(const std::vector<float>& a, const std::vector<float>& b) {
    size_t n = a.size() / 2;
    if (n < 2 || b.size() / 2 < 2) return 1e9f;
    float maxErr = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        float ax = a[i * 2], ay = a[i * 2 + 1];
        float best = 1e9f;
        size_t bn = b.size() / 2;
        for (size_t j = 0; j < bn; ++j) {
            float dx = ax - b[j * 2];
            float dy = ay - b[j * 2 + 1];
            best = fminf(best, sqrtf(dx * dx + dy * dy));
        }
        maxErr = fmaxf(maxErr, best);
    }
    return maxErr;
}

int main() {
    SimParams p{0.5f, 0.0f, 5e30f, 800.0f / 600.0f, 1e8f / 800.0f, 1e8f / 600.0f};

    const float simTime = 1.0f;
    const float refDt = 0.01f;
    const float refSpeed = 0.02f;
    int refSteps = static_cast<int>(simTime / refDt);

    auto ref = runPath(refDt, refSpeed, refSteps, p);

    float maxDtErr = 0.0f;
    for (float dt : {0.005f, 0.01f, 0.02f}) {
        int steps = static_cast<int>(simTime / dt);
        auto path = runPath(dt, refSpeed, steps, p);
        maxDtErr = fmaxf(maxDtErr, maxDiffByArcLength(ref, path));
    }

    // Same arc length per step: speed * dt = refSpeed * refDt => path shape independent of speed.
    float maxSpeedErr = 0.0f;
    for (float speed : {0.01f, 0.02f, 0.04f}) {
        float dt = (refSpeed * refDt) / speed;
        auto path = runPath(dt, speed, refSteps, p);
        maxSpeedErr = fmaxf(maxSpeedErr, maxDiffByArcLength(ref, path));
    }

    const float tol = 1e-3f;
    bool ok = maxDtErr < tol && maxSpeedErr < tol;
    printf("dt sweep max error: %g\n", maxDtErr);
    printf("speed sweep max error: %g\n", maxSpeedErr);
    printf("%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
