// Your First C++ Program

//#include <iostream>

//#include "fouriercurve.cpp"
//typedef xt::xarray<double> CppArray;

#define FORCE_IMPORT_ARRAY
#include "xtensor/xnpy.hpp"
#include "biot_savart.h"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <stdint.h>
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}


int main() {
    //auto mycurve = FourierCurve<CppArray>(10, 3);
    //std::cout << mycurve.gamma()(3, 0) << std::endl;
    //std::cout << mycurve.gamma()(3, 0) << std::endl;
    //mycurve.invalidate_cache();
    //std::cout << mycurve.gamma()(3, 0) << std::endl;
    //mycurve.gamma()(3, 0) = 3.;
    //std::cout << mycurve.gamma()(3, 0) << std::endl;
    //std::cout << "Hello World!";


    xt::xarray<double> points         = xt::load_npy<double>("points_200.npy");
    xt::xarray<double> gamma          = xt::load_npy<double>("gamma_200.npy");
    xt::xarray<double> dgamma_by_dphi = xt::load_npy<double>("dgamma_by_dphi_200.npy");
    int n = 10000;

    auto pointsx = vector_type(points.shape(0), 0);
    auto pointsy = vector_type(points.shape(0), 0);
    auto pointsz = vector_type(points.shape(0), 0);
    for (int i = 0; i < points.shape(0); ++i) {
        pointsx[i] = points(i, 0);
        pointsy[i] = points(i, 1);
        pointsz[i] = points(i, 2);
    }
    uint64_t tick = rdtsc();  // tick before
    auto t1 = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        auto B = xt::xarray<double>::from_shape({points.shape(0), 3});
        std::optional<xt::xarray<double>> dB_by_dX = std::optional(xt::xarray<double>::from_shape({points.shape(0), 3, 3}));
        std::optional<xt::xarray<double>> d2B_by_dXdX = std::optional(xt::xarray<double>::from_shape({points.shape(0), 3, 3, 3}));
        biot_savart_kernel<xt::xarray<double>, 0>(pointsx, pointsy, pointsz, gamma, dgamma_by_dphi, B, dB_by_dX, d2B_by_dXdX);
        if(i==0){
            std::cout << B(0, 0) << " " << B(40, 2) << std::endl;
            std::cout << (*dB_by_dX)(0, 0, 1) << " " << (*dB_by_dX)(40, 2, 2) << std::endl;
            std::cout << (*d2B_by_dXdX)(0, 0, 1, 2) << " " << (*d2B_by_dXdX)(40, 2, 2, 1) << std::endl;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto clockcycles = rdtsc() - tick;
    double simdtime = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "Time: " << simdtime << " ms." << std::endl;
    double interactions = points.shape(0) * gamma.shape(0) * n;
    std::cout << "Gigainteractions per second:" << (interactions/(1e9 * simdtime/1000.)) << std::endl;
    std::cout << "Gigaclockcycles: "<< clockcycles/1e9 << std::endl;
    std::cout << "clockcycles per interaction:" << clockcycles/interactions << std::endl;


    return 0;
}
