#pragma once

#include "izhnet/core/types.hpp"
#include "izhnet/network/network.hpp"

#include <vector>

namespace izhnet {

struct SimulationConfig {
    SimConfig sim {};
    IzhParams neuron {};
    double tonic_current = 0.0;
    double noise_stddev = 0.0;
    std::size_t reserve_spike_events = 0;
};

struct SimulationStats {
    std::uint64_t total_state_updates = 0;
    std::uint64_t total_spikes = 0;
    double elapsed_seconds = 0.0;
    double state_updates_per_second = 0.0;
};

struct SimulationResult {
    NetworkState final_state;
    std::vector<SpikeEvent> spikes;
    SimulationStats stats;
};

SimulationResult simulate_network(const Network& network, NetworkState initial_state, const SimulationConfig& config);

std::vector<SimulationResult> simulate_batch(
    const Network& network,
    const NetworkState& initial_state,
    const std::vector<SimulationConfig>& configs);

} // namespace izhnet
