#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace izhnet 
{

struct IzhParams {
    double V_th = 30.0;    // spike threshold
    double I_e = 0.0;     // constant input current (R=1)
    double V_min = -1.79;   // absolute lower value for the membrane potential
    double a = 0.02;       // describes time scale of recovery variable
    double b = 0.2;       // sensitivity of recovery variable
    double c = -65.0;       // after-spike reset value of V_m
    double d = 8.0;       // after-spike reset value of U_m
    bool consistent_integration = true;    // Use of standard integration technique
};

struct SimConfig {
    double dt_ms = 0.1;
    std::uint32_t steps = 0;
    std::uint64_t seed = 1;
    int omp_threads = 0;
};

struct SpikeEvent {
    std::uint32_t neuron_id;
    std::uint32_t step;
};

struct NetworkState 
{
    std::vector<double> V; // membrane potential
    std::vector<double> U; // recovery variables

    std::vector<double> I;

    std::vector<std::uint8_t> spiked;

    void resize(std::size_t N) {
        V.resize(N);
        U.resize(N);
        I.resize(N);
        spiked.resize(N);
    }

    std::size_t size() const { return V.size(); };
};

inline IzhParams default_params() { return IzhParams{}; }

inline void initial_state(NetworkState& s, std::size_t N, double V_m0 = -65.0, double U_m0 = -13.0, double I0 = 0.0) {
    s.resize(N);
    for (std::size_t i = 0; i < N; ++i) {
        s.V[i] = V_m0;
        s.U[i] = U_m0;
        s.I[i] = I0;
        s.spiked[i] = 0;
    }
}
}