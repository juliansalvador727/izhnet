#include "izhnet/core/types.hpp"
#include "izhnet/io/spike_logger.hpp"
#include "izhnet/network/network.hpp"
#include "izhnet/sim/simulator.hpp"

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

struct CliOptions {
    std::uint32_t n = 1000;
    std::uint32_t steps = 1000;
    std::uint64_t seed = 1;
    std::uint32_t out_degree = 20;
    double dt_ms = 0.1;
    double weight_min = 0.1;
    double weight_max = 2.0;
    double tonic_current = 6.0;
    double noise_stddev = 0.0;
    int omp_threads = 0;
    std::size_t reserve_spikes = 0;
    std::uint32_t sweeps = 1;
    double sweep_current_start = 6.0;
    double sweep_current_step = 0.1;
    bool allow_self_connections = false;
    std::string out_path = "data/spikes.csv";
};

void print_usage(const char* program)
{
    std::cout
        << "Usage: " << program << " [options]\n"
        << "Options:\n"
        << "  --n <int>                    Number of neurons (default: 1000)\n"
        << "  --steps <int>                Number of time steps (default: 1000)\n"
        << "  --dt <float>                 Time step in ms (default: 0.1)\n"
        << "  --seed <int>                 Base RNG seed (default: 1)\n"
        << "  --out <path>                 Output CSV path (default: data/spikes.csv)\n"
        << "  --out-degree <int>           Outgoing edges per neuron (default: 20)\n"
        << "  --w-min <float>              Minimum synaptic weight (default: 0.1)\n"
        << "  --w-max <float>              Maximum synaptic weight (default: 2.0)\n"
        << "  --tonic-current <float>      Constant external current (default: 6.0)\n"
        << "  --noise-stddev <float>       Gaussian current noise sigma (default: 0.0)\n"
        << "  --threads <int>              OpenMP threads; 0 uses runtime default\n"
        << "  --reserve-spikes <int>       Reserve spike events capacity\n"
        << "  --sweeps <int>               Number of parameter sweep runs (default: 1)\n"
        << "  --sweep-current-start <f>    Sweep start current (default: 6.0)\n"
        << "  --sweep-current-step <f>     Sweep current increment (default: 0.1)\n"
        << "  --allow-self-connections     Allow source==target edges\n"
        << "  --help                       Show this help\n";
}

std::string require_value(int argc, char** argv, int& index, const std::string& option)
{
    if (index + 1 >= argc) {
        throw std::invalid_argument("missing value for " + option);
    }
    ++index;
    return std::string(argv[index]);
}

std::uint32_t parse_u32(const std::string& text, const std::string& option)
{
    const unsigned long long value = std::stoull(text);
    if (value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument(option + " out of range for uint32");
    }
    return static_cast<std::uint32_t>(value);
}

std::uint64_t parse_u64(const std::string& text, const std::string& option)
{
    (void)option;
    return std::stoull(text);
}

int parse_int(const std::string& text, const std::string& option)
{
    const long value = std::stol(text);
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        throw std::invalid_argument(option + " out of range for int");
    }
    return static_cast<int>(value);
}

std::size_t parse_size(const std::string& text, const std::string& option)
{
    (void)option;
    return static_cast<std::size_t>(std::stoull(text));
}

double parse_double(const std::string& text, const std::string& option)
{
    (void)option;
    return std::stod(text);
}

enum class ParseResult {
    Ok,
    Help
};

ParseResult parse_args(int argc, char** argv, CliOptions& options)
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);

        if (arg == "--help") {
            return ParseResult::Help;
        }
        if (arg == "--allow-self-connections") {
            options.allow_self_connections = true;
            continue;
        }
        if (arg == "--n") {
            options.n = parse_u32(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--steps") {
            options.steps = parse_u32(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--dt") {
            options.dt_ms = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--seed") {
            options.seed = parse_u64(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--out") {
            options.out_path = require_value(argc, argv, i, arg);
            continue;
        }
        if (arg == "--out-degree") {
            options.out_degree = parse_u32(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--w-min") {
            options.weight_min = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--w-max") {
            options.weight_max = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--tonic-current") {
            options.tonic_current = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--noise-stddev") {
            options.noise_stddev = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--threads") {
            options.omp_threads = parse_int(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--reserve-spikes") {
            options.reserve_spikes = parse_size(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--sweeps") {
            options.sweeps = parse_u32(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--sweep-current-start") {
            options.sweep_current_start = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }
        if (arg == "--sweep-current-step") {
            options.sweep_current_step = parse_double(require_value(argc, argv, i, arg), arg);
            continue;
        }

        throw std::invalid_argument("unknown option: " + arg);
    }

    if (options.n == 0) {
        throw std::invalid_argument("--n must be > 0");
    }
    if (options.steps == 0) {
        throw std::invalid_argument("--steps must be > 0");
    }
    if (options.dt_ms <= 0.0) {
        throw std::invalid_argument("--dt must be > 0");
    }
    if (options.weight_min > options.weight_max) {
        throw std::invalid_argument("--w-min must be <= --w-max");
    }
    if (options.noise_stddev < 0.0) {
        throw std::invalid_argument("--noise-stddev must be >= 0");
    }
    if (options.omp_threads < 0) {
        throw std::invalid_argument("--threads must be >= 0");
    }
    if (options.sweeps == 0) {
        throw std::invalid_argument("--sweeps must be > 0");
    }

    return ParseResult::Ok;
}

std::filesystem::path output_path_for_run(const std::string& out_path, std::uint32_t run_index, std::uint32_t run_count)
{
    const std::filesystem::path base(out_path);
    if (run_count <= 1) {
        return base;
    }

    if (base.extension().empty()) {
        std::ostringstream filename;
        filename << "spikes_run_" << std::setw(4) << std::setfill('0') << run_index << ".csv";
        return base / filename.str();
    }

    std::ostringstream suffix;
    suffix << "_run_" << std::setw(4) << std::setfill('0') << run_index;
    const std::filesystem::path parent = base.has_parent_path() ? base.parent_path() : std::filesystem::path(".");
    return parent / (base.stem().string() + suffix.str() + base.extension().string());
}

} // namespace

int main(int argc, char** argv)
{
    try {
        CliOptions options;
        const ParseResult parse_result = parse_args(argc, argv, options);
        if (parse_result == ParseResult::Help) {
            print_usage(argv[0]);
            return 0;
        }

        const izhnet::Network network = izhnet::Network::random_fixed_out_degree(
            options.n,
            options.out_degree,
            options.weight_min,
            options.weight_max,
            options.seed,
            options.allow_self_connections);

        izhnet::NetworkState initial;
        izhnet::initial_state(initial, options.n, -65.0, -13.0, 0.0);

        izhnet::SimulationConfig base_config;
        base_config.sim.dt_ms = options.dt_ms;
        base_config.sim.steps = options.steps;
        base_config.sim.seed = options.seed;
        base_config.sim.omp_threads = options.omp_threads;
        base_config.tonic_current = options.tonic_current;
        base_config.noise_stddev = options.noise_stddev;
        base_config.reserve_spike_events = options.reserve_spikes;

        std::uint64_t total_spikes = 0;
        std::uint64_t total_updates = 0;
        double total_elapsed_s = 0.0;

        for (std::uint32_t run = 0; run < options.sweeps; ++run) {
            izhnet::SimulationConfig run_config = base_config;
            if (options.sweeps > 1) {
                run_config.sim.seed = options.seed + run;
                run_config.tonic_current = options.sweep_current_start + options.sweep_current_step * static_cast<double>(run);
            }

            izhnet::SimulationResult result = izhnet::simulate_network(network, initial, run_config);

            const std::filesystem::path run_output = output_path_for_run(options.out_path, run, options.sweeps);
            const izhnet::SpikeLogSummary summary =
                izhnet::write_spikes_csv(run_output.string(), result.spikes, run_config.sim.dt_ms, true);

            total_spikes += result.stats.total_spikes;
            total_updates += result.stats.total_state_updates;
            total_elapsed_s += result.stats.elapsed_seconds;

            std::cout
                << "run=" << run
                << " out=" << run_output.string()
                << " spikes=" << summary.events_written
                << " duration_ms=" << summary.duration_ms
                << " updates_per_s=" << std::fixed << std::setprecision(3) << result.stats.state_updates_per_second
                << "\n";
        }

        const double aggregate_updates_per_s = (total_elapsed_s > 0.0)
            ? (static_cast<double>(total_updates) / total_elapsed_s)
            : 0.0;

        std::cout
            << "summary runs=" << options.sweeps
            << " total_spikes=" << total_spikes
            << " total_state_updates=" << total_updates
            << " aggregate_updates_per_s=" << std::fixed << std::setprecision(3) << aggregate_updates_per_s
            << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        std::cerr << "use --help for options\n";
        return 1;
    }
}
