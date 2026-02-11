#include "izhnet/model/izhikevich.hpp"

#include <algorithm>

namespace izhnet {

static inline double dv_dt(double V, double U, double I) { return 0.04 * (V*V) + (5*V) + 140 - U + I; }
static inline double du_dt(const IzhParams& p, double V, double U) {return p.a * (p.b * V - U); }

bool step_izhikevich(double& V, double& U, double I, double dt_ms, const IzhParams& p)
{
    bool spiked{ false };
    // gotta set this up fr
    if (p.consistent_integration) {
        // dont use nest
    } else {
        // use nest
    }

    (void)I;
    (void)dt_ms;
    (void)p;
    V = std::max(V, p.V_min);
    return false;
}

}

