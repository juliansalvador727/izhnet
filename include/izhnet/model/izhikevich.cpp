#include "izhnet/model/izhikevich.hpp"

#include <algorithm>

namespace izhnet {

bool step_izhikevich(double& V, double& U, double I, double dt_ms, const IzhParams& p)
{
    // gotta set this up fr

    (void)I;
    (void)dt_ms;
    (void)p;
    V = std::max(V, p.V_min);
    return false;
}

}

