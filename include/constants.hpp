#pragma once

// Unit system: geometric / world units (G = c = 1 for Schwarzschild formulas).
// 1 world unit = 1 length in the simulation; render mesh radii use the same numbers.
// SI values (G_SI, c_SI) are kept only for optional conversion or weak-field checks.

namespace constants {

// Black hole (geometric units: rs = 2M) 
constexpr double M = 0.175;
constexpr double rs = 2.0 * M;  // 0.35 — keep equal to visual sphere radius in main

// Critical impact parameter for capture: photon sphere grazing
constexpr double bCritical() {
    return 1.5 * 1.7320508075688772 * rs;  // (3*sqrt(3)/2) * rs
}

// Weak-field deflection angle (radians) at large b: alpha ≈ 2*rs/b
inline double weakFieldDeflection(double b) {
    return 2.0 * rs / b;
}

// Geodesic integration (equatorial plane, u = 1/r)
constexpr double rStart = 20.0;       // large distance to approximate "infinity"
constexpr double rMax = 100.0;        // stop if photon escapes beyond this
constexpr double dPhi = 0.0001;        // integration step in radians (tune for accuracy/speed)
constexpr int maxGeodesicSteps = 2000000;
constexpr int geodesicStepsPerFrame = 32;

// Scene layout (world units, must match main / mesh setup)
constexpr float bhRenderRadius = static_cast<float>(rs);

// Multi-direction ray fan (approach angle = velocity at infinity in xy plane)
constexpr int rayDirectionCount = 3;   // evenly spaced azimuths around the BH
constexpr double rayBMin = 0.25;
constexpr double rayBMax = 3.5;
constexpr double rayBStep = 0.35;
constexpr int rayTiltCount = 2;        // orbital planes: -tilt, 0, +tilt about X
constexpr double rayTiltMax = 0.1;    // radians (~31°)

}
