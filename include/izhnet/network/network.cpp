#include "izhnet/network/network.hpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>

namespace izhnet {

Network::Network(std::uint32_t neuron_count)
    : neuron_count_(neuron_count) {}

void Network::resize(std::uint32_t neuron_count)
{
    neuron_count_ = neuron_count;
    clear_edges();
}

std::uint32_t Network::size() const
{
    return neuron_count_;
}

void Network::clear_edges()
{
    edges_.clear();
    offsets_.clear();
    targets_.clear();
    weights_.clear();
    finalized_ = false;
}

void Network::reserve_edges(std::size_t edge_count)
{
    edges_.reserve(edge_count);
}

void Network::add_edge(std::uint32_t source, std::uint32_t target, double weight)
{
    if (finalized_) {
        throw std::logic_error("add_edge called after finalize; call clear_edges first");
    }
    if (source >= neuron_count_ || target >= neuron_count_) {
        throw std::out_of_range("edge endpoint out of range");
    }
    edges_.push_back(Edge { source, target, weight });
}

void Network::finalize()
{
    offsets_.assign(static_cast<std::size_t>(neuron_count_) + 1U, 0U);
    for (const Edge& edge : edges_) {
        if (edge.source >= neuron_count_ || edge.target >= neuron_count_) {
            throw std::out_of_range("edge endpoint out of range");
        }
        ++offsets_[static_cast<std::size_t>(edge.source) + 1U];
    }
    std::partial_sum(offsets_.begin(), offsets_.end(), offsets_.begin());

    targets_.assign(edges_.size(), 0U);
    weights_.assign(edges_.size(), 0.0);

    std::vector<std::uint32_t> cursor = offsets_;
    for (const Edge& edge : edges_) {
        const std::size_t idx = cursor[edge.source]++;
        targets_[idx] = edge.target;
        weights_[idx] = edge.weight;
    }

    edges_.clear();
    edges_.shrink_to_fit();
    finalized_ = true;
}

bool Network::is_finalized() const
{
    return finalized_;
}

std::size_t Network::edge_count() const
{
    return finalized_ ? targets_.size() : edges_.size();
}

const std::vector<std::uint32_t>& Network::offsets() const
{
    return offsets_;
}

const std::vector<std::uint32_t>& Network::targets() const
{
    return targets_;
}

const std::vector<double>& Network::weights() const
{
    return weights_;
}

Network Network::random_fixed_out_degree(
    std::uint32_t neuron_count,
    std::uint32_t out_degree,
    double weight_min,
    double weight_max,
    std::uint64_t seed,
    bool allow_self_connections)
{
    Network network(neuron_count);
    if (neuron_count == 0 || out_degree == 0) {
        network.finalize();
        return network;
    }

    network.reserve_edges(static_cast<std::size_t>(neuron_count) * out_degree);

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint32_t> target_dist(0U, neuron_count - 1U);
    std::uniform_real_distribution<double> weight_dist(weight_min, weight_max);

    for (std::uint32_t source = 0; source < neuron_count; ++source) {
        for (std::uint32_t j = 0; j < out_degree; ++j) {
            std::uint32_t target = target_dist(rng);
            if (!allow_self_connections && neuron_count > 1U) {
                while (target == source) {
                    target = target_dist(rng);
                }
            }
            network.add_edge(source, target, weight_dist(rng));
        }
    }

    network.finalize();
    return network;
}

} // namespace izhnet
