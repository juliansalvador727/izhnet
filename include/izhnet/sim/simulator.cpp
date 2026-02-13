#include "izhnet/sim/simulator.hpp"

#include "izhnet/model/izhikevich.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

#if IZHNET_HAS_OPENMP
#include <omp.h>
#endif

namespace izhnet {

SimulationResult simulate_network(const Network& network, NetworkState initial_state, const SimulationConfig& config)
{
    if (!network.is_finalized()) {
        throw std::invalid_argument("network must be finalized before simulation");
    }

    const std::size_t neuron_count = network.size();
    if (initial_state.size() != neuron_count) {
        throw std::invalid_argument("initial_state size must match network size");
    }

    SimulationResult result;
    result.final_state = std::move(initial_state);
    if (config.reserve_spike_events > 0) {
        result.spikes.reserve(config.reserve_spike_events);
    }

    std::vector<double> syn_current(neuron_count, 0.0);
    std::vector<double> next_syn_current(neuron_count, 0.0);

    std::mt19937_64 rng(config.sim.seed);
    std::normal_distribution<double> noise_dist(0.0, config.noise_stddev);

#if IZHNET_HAS_OPENMP
    if (config.sim.omp_threads > 0) {
        omp_set_num_threads(config.sim.omp_threads);
    }
    const bool can_parallel = (config.noise_stddev <= 0.0) && (neuron_count >= 1024U);
    std::vector<std::vector<std::uint32_t>> thread_spikes;
    if (can_parallel) {
        thread_spikes.resize(static_cast<std::size_t>(omp_get_max_threads()));
        if (config.reserve_spike_events > 0 && !thread_spikes.empty()) {
            const std::size_t per_thread = config.reserve_spike_events / thread_spikes.size();
            for (auto& local : thread_spikes) {
                local.reserve(per_thread);
            }
        }
    }
#else
    const bool can_parallel = false;
#endif

    const auto t0 = std::chrono::steady_clock::now();
    const auto& offsets = network.offsets();
    const auto& targets = network.targets();
    const auto& weights = network.weights();

    for (std::uint32_t step = 0; step < config.sim.steps; ++step) {
        if (can_parallel) {
#if IZHNET_HAS_OPENMP
            for (auto& local : thread_spikes) {
                local.clear();
            }
#pragma omp parallel
            {
                const int tid = omp_get_thread_num();
                auto& local = thread_spikes[static_cast<std::size_t>(tid)];
#pragma omp for schedule(static)
                for (std::int64_t i = 0; i < static_cast<std::int64_t>(neuron_count); ++i) {
                    const std::size_t idx = static_cast<std::size_t>(i);
                    const double total_current = result.final_state.I[idx] + config.tonic_current + syn_current[idx];

                    const bool spiked = step_izhikevich(
                        result.final_state.V[idx],
                        result.final_state.U[idx],
                        total_current,
                        config.sim.dt_ms,
                        config.neuron);

                    result.final_state.spiked[idx] = static_cast<std::uint8_t>(spiked ? 1U : 0U);
                    if (spiked) {
                        local.push_back(static_cast<std::uint32_t>(idx));
                    }
                }
            }

            for (const auto& local : thread_spikes) {
                for (const std::uint32_t neuron_id : local) {
                    result.spikes.push_back(SpikeEvent { neuron_id, step });
                }
            }
#endif
        } else {
            for (std::size_t i = 0; i < neuron_count; ++i) {
                double total_current = result.final_state.I[i] + config.tonic_current + syn_current[i];
                if (config.noise_stddev > 0.0) {
                    total_current += noise_dist(rng);
                }

                const bool spiked = step_izhikevich(
                    result.final_state.V[i],
                    result.final_state.U[i],
                    total_current,
                    config.sim.dt_ms,
                    config.neuron);

                result.final_state.spiked[i] = static_cast<std::uint8_t>(spiked ? 1U : 0U);
                if (spiked) {
                    result.spikes.push_back(SpikeEvent {
                        static_cast<std::uint32_t>(i),
                        step
                    });
                }
            }
        }

        std::fill(next_syn_current.begin(), next_syn_current.end(), 0.0);
        for (std::size_t source = 0; source < neuron_count; ++source) {
            if (!result.final_state.spiked[source]) {
                continue;
            }
            const std::size_t edge_begin = offsets[source];
            const std::size_t edge_end = offsets[source + 1U];
            for (std::size_t edge_idx = edge_begin; edge_idx < edge_end; ++edge_idx) {
                next_syn_current[targets[edge_idx]] += weights[edge_idx];
            }
        }
        syn_current.swap(next_syn_current);
    }

    const auto t1 = std::chrono::steady_clock::now();
    result.stats.elapsed_seconds = std::chrono::duration<double>(t1 - t0).count();
    result.stats.total_spikes = result.spikes.size();
    result.stats.total_state_updates = static_cast<std::uint64_t>(2ULL) *
        static_cast<std::uint64_t>(neuron_count) *
        static_cast<std::uint64_t>(config.sim.steps);

    if (result.stats.elapsed_seconds > 0.0) {
        result.stats.state_updates_per_second =
            static_cast<double>(result.stats.total_state_updates) / result.stats.elapsed_seconds;
    } else {
        result.stats.state_updates_per_second = std::numeric_limits<double>::infinity();
    }

    return result;
}

std::vector<SimulationResult> simulate_batch(
    const Network& network,
    const NetworkState& initial_state,
    const std::vector<SimulationConfig>& configs)
{
    std::vector<SimulationResult> results;
    results.reserve(configs.size());
    for (const SimulationConfig& config : configs) {
        results.push_back(simulate_network(network, initial_state, config));
    }
    return results;
}

} // namespace izhnet
