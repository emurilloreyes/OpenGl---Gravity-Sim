#include "geodesic.hpp"

#include <cmath>

#include "constants.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GeodesicState geodesicDerivative(const GeodesicState& s, double rs) {
    GeodesicState d;
    d.u = s.uPrime;
    d.uPrime = 1.5 * rs * s.u * s.u - s.u;
    d.phi = 1.0;
    return d;
}

static GeodesicState addState(const GeodesicState& a, const GeodesicState& b, double scale) {
    return {
        a.u + scale * b.u,
        a.uPrime + scale * b.uPrime,
        a.phi + scale * b.phi
    };
}

GeodesicState rk4Step(const GeodesicState& s, double dPhi, double rs) {
    const GeodesicState k1 = geodesicDerivative(s, rs);
    const GeodesicState k2 = geodesicDerivative(addState(s, k1, 0.5 * dPhi), rs);
    const GeodesicState k3 = geodesicDerivative(addState(s, k2, 0.5 * dPhi), rs);
    const GeodesicState k4 = geodesicDerivative(addState(s, k3, dPhi), rs);

    GeodesicState sum = addState(k1, k2, 1.0);
    sum = addState(sum, k2, 1.0);
    sum = addState(sum, k3, 2.0);
    sum = addState(sum, k4, 1.0);
    return addState(s, sum, dPhi / 6.0);
}

Vec3 geodesicToCartesian(const GeodesicState& s) {
    const double r = 1.0 / s.u;
    return {
        static_cast<float>(r * std::cos(s.phi)),
        static_cast<float>(r * std::sin(s.phi)),
        0.0f
    };
}

double geodesicRadius(const GeodesicState& s) {
    if (s.u <= 0.0) return 0.0;
    return 1.0 / s.u;
}

void Photon::appendCurrentPosition() {
    Vec3 p = geodesicToCartesian(state);
    if (planeTiltX != 0.0) {
        const float c = static_cast<float>(std::cos(planeTiltX));
        const float s = static_cast<float>(std::sin(planeTiltX));
        const float y = p.y * c;
        const float z = p.y * s;
        p.y = y;
        p.z = z;
    }
    path.push_back(p.x);
    path.push_back(p.y);
    path.push_back(p.z);
}

void Photon::resetFromDirection(double b, double approachAngle, double planeTiltX_, double rStart) {
    const double r0 = (rStart > 0.0) ? rStart : constants::rStart;
    impactParameter = b;
    planeTiltX = planeTiltX_;
    state.u = 1.0 / r0;
    state.uPrime = 1.0 / b;
    state.phi = approachAngle + M_PI;
    end = GeodesicEnd::Active;
    path.clear();
    appendCurrentPosition();
}

void Photon::resetFromImpactParameter(double b, double rStart) {
    resetFromDirection(b, 0.0, 0.0, rStart);
}

bool Photon::step(double dPhi, double rs, double rMax) {
    if (!isActive()) return false;

    state = rk4Step(state, dPhi, rs);
    appendCurrentPosition();

    const double r = geodesicRadius(state);
    if (r <= rs) {
        end = GeodesicEnd::Captured;
        return false;
    }
    if (r >= rMax || state.u <= 0.0) {
        end = GeodesicEnd::Escaped;
        return false;
    }
    return true;
}

std::vector<float> traceEquatorialGeodesic(double b, double rs,
                                           double rStart, double rMax,
                                           double dPhi, int maxSteps,
                                           GeodesicEnd& endOut) {
    Photon photon;
    photon.resetFromImpactParameter(b, rStart);

    for (int i = 0; i < maxSteps && photon.isActive(); ++i) {
        photon.step(dPhi, rs, rMax);
    }

    endOut = photon.end;
    if (endOut == GeodesicEnd::Active) {
        endOut = GeodesicEnd::Escaped;
    }
    return photon.path;
}
