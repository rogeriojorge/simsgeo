#pragma once

#define BLAZE_USE_SHARED_MEMORY_PARALLELIZATION 0

#include "xtensor/xio.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xmath.hpp"
#include "blaze/Blaze.h"
#include "xtensor-python/pyarray.hpp"     // Numpy bindings
#include <tuple>


typedef blaze::StaticVector<double,3UL> Vec3d;
typedef blaze::DynamicMatrix<double, blaze::rowMajor> RowMat;
typedef blaze::DynamicMatrix<double, blaze::columnMajor> ColMat;

typedef xt::pyarray<double> Array;


#include <vector>
using std::vector;

#include "xsimd/xsimd.hpp"
namespace xs = xsimd;
using xs::sqrt;
using vector_type = std::vector<double, xs::aligned_allocator<double, XSIMD_DEFAULT_ALIGNMENT>>;
using simd_t = xs::simd_type<double>;
#include <optional>


struct Vec3dSimd {
    simd_t x;
    simd_t y;
    simd_t z;

    Vec3dSimd() : x(0.), y(0.), z(0.){
    }

    Vec3dSimd(double x_, double y_, double z_) : x(x_), y(y_), z(z_){
    }

    Vec3dSimd(Vec3d xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]){
    }

    Vec3dSimd(const simd_t& x_, const simd_t& y_, const simd_t& z_) : x(x_), y(y_), z(z_) {
    }

    Vec3dSimd(double* xptr, double* yptr, double *zptr){
        x = xs::load_aligned(xptr);
        y = xs::load_aligned(yptr);
        z = xs::load_aligned(zptr);
    }

    void store_aligned(double* xptr, double* yptr, double *zptr){
        x.store_aligned(xptr);
        y.store_aligned(yptr);
        z.store_aligned(zptr);
    }

    simd_t& operator[] (int i){
        if(i==0) {
            return x;
        }else if(i==1){
            return y;
        } else{
            return z;
        }
    }

    friend Vec3dSimd operator+(Vec3dSimd lhs, const Vec3d& rhs) {
        lhs.x += rhs[0];
        lhs.y += rhs[1];
        lhs.z += rhs[2];
        return lhs;
    }

    friend Vec3dSimd operator+(Vec3dSimd lhs, const Vec3dSimd& rhs) {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        lhs.z += rhs.z;
        return lhs;
    }

    Vec3dSimd& operator+=(const Vec3dSimd& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
        return *this;
    }

    Vec3dSimd& operator-=(const Vec3dSimd& rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;
        return *this;
    }

    friend Vec3dSimd operator-(Vec3dSimd lhs, const Vec3d& rhs) {
        lhs.x -= rhs[0];
        lhs.y -= rhs[1];
        lhs.z -= rhs[2];
        return lhs;
    }

    friend Vec3dSimd operator-(Vec3dSimd lhs, const Vec3dSimd& rhs) {
        lhs.x -= rhs.x;
        lhs.y -= rhs.y;
        lhs.z -= rhs.z;
        return lhs;
    }

    friend Vec3dSimd operator*(Vec3dSimd lhs, const simd_t& rhs) {
        lhs.x *= rhs;
        lhs.y *= rhs;
        lhs.z *= rhs;
        return lhs;
    }
};


inline simd_t inner(const Vec3dSimd& a, const Vec3dSimd& b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

inline simd_t inner(const Vec3d& b, const Vec3dSimd& a){
    return a.x*b[0]+a.y*b[1]+a.z*b[2];
}

inline simd_t inner(const Vec3dSimd& a, const Vec3d& b){
    return a.x*b[0]+a.y*b[1]+a.z*b[2];
}

inline simd_t inner(int i, Vec3dSimd& a){
    if(i==0)
        return a.x;
    else if(i==1)
        return a.y;
    else
        return a.z;
}


inline Vec3dSimd cross(Vec3dSimd& a, Vec3dSimd& b){
    return Vec3dSimd(
            xsimd::fms(a.y, b.z, a.z * b.y),
            xsimd::fms(a.z, b.x, a.x * b.z),
            xsimd::fms(a.x, b.y, a.y * b.x)
            );
}

inline Vec3dSimd cross(Vec3dSimd& a, Vec3d& b){
    return Vec3dSimd(a.y * b[2] - a.z * b[1], a.z * b[0] - a.x * b[2], a.x * b[1] - a.y * b[0]);

}

inline Vec3dSimd cross(Vec3d& a, Vec3dSimd& b){
    return Vec3dSimd(a[1] * b.z - a[2] * b.y, a[2] * b.x - a[0] * b.z, a[0] * b.y - a[1] * b.x);
}

inline Vec3dSimd cross(Vec3dSimd& a, int i){
    if(i==0)
        return Vec3dSimd(simd_t(0.), a.z, -a.y);
    else if(i == 1)
        return Vec3dSimd(-a.z, simd_t(0.), a.x);
    else
        return Vec3dSimd(a.y, -a.x, simd_t(0.));
}

inline Vec3dSimd cross(int i, Vec3dSimd& b){
    if(i==0)
        return Vec3dSimd(simd_t(0.), -b.z, b.y);
    else if(i == 1)
        return Vec3dSimd(b.z, simd_t(0.), -b.x);
    else
        return Vec3dSimd(-b.y, b.x, simd_t(0.));
}

inline simd_t normsq(Vec3dSimd& a){
    return a.x*a.x+a.y*a.y+a.z*a.z;
}

template<class T, int derivs>
void biot_savart_kernel(vector_type& pointsx, vector_type& pointsy, vector_type& pointsz, T& gamma, T& dgamma_by_dphi, T& B, std::optional<T>& dB_by_dX, std::optional<T>& d2B_by_dXdX);
void biot_savart(Array& points, vector<Array>& gammas, vector<Array>& dgamma_by_dphis, vector<Array>& B, vector<Array>& dB_by_dX, vector<Array>& d2B_by_dXdX);



template<class T>
void biot_savart_B_only_vjp_impl(vector_type& pointsx, vector_type& pointsy, vector_type& pointsz, T& gamma, T& dgamma_by_dphi, T& v, T& res_gamma, T& res_dgamma_by_dphi, T& vgrad, T& res_grad_gamma, T& res_grad_dgamma_by_dphi);
void biot_savart_by_dcoilcoeff_all_vjp(Array& points, vector<Array>& gammas, vector<Array>& dgamma_by_dphis, vector<double>& currents, Array& v, vector<Array>& res_gamma, vector<Array>& res_dgamma_by_dphi, Array& vgrad, vector<Array>& res_grad_gamma, vector<Array>& res_grad_dgamma_by_dphi);
void biot_savart_by_dcoilcoeff_all_vjp_full(Array& points, vector<Array>& gammas, vector<Array>& dgamma_by_dphis, vector<double>& currents, Array& v, Array& vgrad, vector<Array>& dgamma_by_dcoeffs, vector<Array>& d2gamma_by_dphidcoeffs, vector<Array>& res_B, vector<Array>& res_dB);
