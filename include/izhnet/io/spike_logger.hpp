#pragma once

#include "izhnet/core/types.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace izhnet {

struct SpikeLogSummary {
    std::size_t events_written = 0;
    double duration_ms = 0.0;
};

SpikeLogSummary write_spikes_csv(
    const std::string& output_path,
    const std::vector<SpikeEvent>& spikes,
    double dt_ms,
    bool include_header = true);

} // namespace izhnet
