#include "izhnet/model/izhikevich.hpp"

#include <algorithm>

namespace izhnet {

static inline double dv_dt(double V, double U, double I) { return 0.04 * (V*V) + (5*V) + 140 - U + I; }
static inline double du_dt(const IzhParams& p, double V, double U) {return p.a * (p.b * V - U); }

bool step_izhikevich(double& V, double& U, double I, double dt_ms, const IzhParams& p)
{
    if (p.consistent_integration) {
        // Standard explicit Euler integration.
        const double dV = dv_dt(V, U, I);
        const double dU = du_dt(p, V, U);
        V += dt_ms * dV;
        U += dt_ms * dU;
    } else {
        // Published scheme: two V half-steps, then U from updated V.
        V += 0.5 * dt_ms * dv_dt(V, U, I);
        V += 0.5 * dt_ms * dv_dt(V, U, I);
        U += dt_ms * du_dt(p, V, U);
    }

    V = std::max(V, p.V_min);
    if (V >= p.V_th) {
        V = p.c;
        U += p.d;
        return true;
    }

    return false;
}

}
