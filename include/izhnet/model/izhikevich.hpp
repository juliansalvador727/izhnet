#pragma once
#include "izhnet/core/types.hpp"

namespace izhnet {

bool step_izhikevich(double& V, double& U, double I, double dt_ms, const IzhParams& p);

}

