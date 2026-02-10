#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace izhnet 
{

struct Params {
    double V_m;     // membrane potential
    double U_m;     // membrane potential recovery variable
    double V_th;    // spike threshold
    double I_e;     // constant input current (R=1)
    double V_min;   // absolute lower value for the membrane potential
    double a;       // describes time scale of recovery variable
    double b;       // sensitivity of recovery variable
    double c;       // after-spike reset value of V_m
    double d;       // after-spike reset value of U_m
    bool consistent_integration;    // Use of standard integration technique
};

inline Params default_params() 
{
    return {
        -65.0, // V_m
        -13.0, // U_m
        30.0,  // V_th
        0.0,   // I_e
        -1.79, // V_min
        0.02,  // a
        0.2,   // b
        -65.0, // c
        8.0,   // d
        true   // consistent_integration
    };
}

}