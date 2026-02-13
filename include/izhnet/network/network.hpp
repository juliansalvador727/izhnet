#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace izhnet {

struct Edge {
    std::uint32_t source = 0;
    std::uint32_t target = 0;
    double weight = 0.0;
};

class Network {
public:
    explicit Network(std::uint32_t neuron_count = 0);

    void resize(std::uint32_t neuron_count);
    std::uint32_t size() const;

    void clear_edges();
    void reserve_edges(std::size_t edge_count);
    void add_edge(std::uint32_t source, std::uint32_t target, double weight);
    void finalize();

    bool is_finalized() const;
    std::size_t edge_count() const;

    const std::vector<std::uint32_t>& offsets() const;
    const std::vector<std::uint32_t>& targets() const;
    const std::vector<double>& weights() const;

    static Network random_fixed_out_degree(
        std::uint32_t neuron_count,
        std::uint32_t out_degree,
        double weight_min,
        double weight_max,
        std::uint64_t seed,
        bool allow_self_connections = false);

private:
    std::uint32_t neuron_count_{ 0 };
    bool finalized_{ false };
    std::vector<Edge> edges_;
    std::vector<std::uint32_t> offsets_;
    std::vector<std::uint32_t> targets_;
    std::vector<double> weights_;
};

} // namespace izhnet
