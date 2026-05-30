#pragma once

#include <vector>
#include <cstddef>

#include "math.hpp"

enum class GeodesicEnd {
    Active,
    Captured,
    Escaped
};

struct GeodesicState {
    double u = 0.0;       // 1/r
    double uPrime = 0.0;  // du/dφ
    double phi = 0.0;
};

// Schwarzschild null geodesic in equatorial plane: u'' + u = (3/2) rs u^2
GeodesicState geodesicDerivative(const GeodesicState& s, double rs);

GeodesicState rk4Step(const GeodesicState& s, double dPhi, double rs);

Vec3 geodesicToCartesian(const GeodesicState& s);

double geodesicRadius(const GeodesicState& s);

// Integrate from impact parameter b until capture, escape, or step limit.
std::vector<float> traceEquatorialGeodesic(double b, double rs,
                                           double rStart, double rMax,
                                           double dPhi, int maxSteps,
                                           GeodesicEnd& endOut);

class Photon {
public:
    GeodesicState state{};
    std::vector<float> path;
    GeodesicEnd end = GeodesicEnd::Active;
    double impactParameter = 1.0;
    double planeTiltX = 0.0;  // rotate orbital plane about X (radians)

    void resetFromImpactParameter(double b, double rStart = 0.0);
  // approachAngle: velocity direction at infinity in equatorial plane; planeTiltX: incline orbit
    void resetFromDirection(double b, double approachAngle, double planeTiltX = 0.0, double rStart = 0.0);
    bool step(double dPhi, double rs, double rMax);

    bool isActive() const { return end == GeodesicEnd::Active; }

    void appendCurrentPosition();
};
