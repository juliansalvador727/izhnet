#include "izhnet/io/spike_logger.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace izhnet {

SpikeLogSummary write_spikes_csv(
    const std::string& output_path,
    const std::vector<SpikeEvent>& spikes,
    double dt_ms,
    bool include_header)
{
    if (dt_ms <= 0.0) {
        throw std::invalid_argument("dt_ms must be > 0");
    }

    const std::filesystem::path path(output_path);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream out(output_path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("failed to open spike csv for writing: " + output_path);
    }

    if (include_header) {
        out << "time_ms,neuron_id,step\n";
    }
    out << std::fixed << std::setprecision(3);

    std::uint32_t max_step = 0;
    for (const SpikeEvent& event : spikes) {
        out << (static_cast<double>(event.step) * dt_ms) << "," << event.neuron_id << "," << event.step << "\n";
        max_step = std::max(max_step, event.step);
    }

    out.flush();
    if (!out.good()) {
        throw std::runtime_error("failed while writing spike csv: " + output_path);
    }

    return SpikeLogSummary {
        spikes.size(),
        static_cast<double>(max_step) * dt_ms
    };
}

} // namespace izhnet
