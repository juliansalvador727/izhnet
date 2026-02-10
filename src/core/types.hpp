#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace izhnet 
{

struct Params {
    double V_m;
    double U_m;
    double V_th;
    double I_e;
    double V_min;
    double a;
    double b;
    double c;
    double d;
    bool consistent_integration;
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