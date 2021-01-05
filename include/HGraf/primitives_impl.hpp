#pragma once

#include "primitives.hpp"

namespace hg {

    /*
     * RangeN member functions
     */

    // Constructors

    template<int n>
    RangeN<n>::RangeN(const falg::Matrix<n, 1>& minp, const falg::Matrix<n, 1>& maxp) {
        this->min = minp;
        this->max = maxp;
    }

    template<int n>
    RangeN<n> RangeN<n>::fromMiddleRange(const falg::Matrix<n, 1>& middle, const falg::Matrix<n, 1>& range) {
        return RangeN<n>(middle - range, middle + range);
    }


    // Getters

    template<int n>
    falg::Matrix<n, 1> RangeN<n>::getMin() const {
        return this->min;
    }

    template<int n>
    falg::Matrix<n, 1> RangeN<n>::getMax() const {
        return this->max;
    }

    template<int n>
    const falg::Matrix<n, 1>& RangeN<n>::getMiddle() const {
        return (this->min + this->max) / 2;
    }

    template<int n>
    const falg::Matrix<n, 1>& RangeN<n>::getRange() const {
        return (this->max - this->min) / 2;
    }

    template<int n>
    const falg::Matrix<n, 1>& RangeN<n>::getDims() const {
        return this->max - this->min;
    }

    template<int n>
    falg::Matrix<n, 1>& RangeN<n>::getMin() {
        return this->min;
    }

    template<int n>
    falg::Matrix<n, 1>& RangeN<n>::getMax() {
        return this->max;
    }

    // Modifiers

    template<int n>
    void RangeN<n>::add(const falg::Matrix<n, 1>& p) {
        for (int i = 0; i < n; i++) {
            this->min[i] = std::min(this->min[i], p[i]);
            this->max[i] = std::max(this->max[i], p[i]);
        }
    }

    // Queries

    template<int n>
    bool RangeN<n>::contains(const falg::Matrix<n, 1>& p) const {
        bool outside = false;
        for (int i = 0; i < n; i++) {
            outside = outside || p[i] < this->min[i] || p[i] > this->max[i];
        }

        return outside;
    }

    /*
     * SphereN member functions
     */

    // Constructors

    template<int n>
    SphereN<n>::SphereN(const falg::Matrix<n, 1>& center, float r) : center(center), r(r) { }

    // Getters

    template<int n>
    const float& SphereN<n>::getRadius() const {
        return this->r;
    }

    template<int n>
    float& SphereN<n>::getRadius() {
        return this->r;
    }

    template<int n>
    falg::Matrix<n, 1>& SphereN<n>::getCenter() {
        return this->center;
    }

    template<int n>
    const falg::Matrix<n, 1>& SphereN<n>::getCenter() const {
        return this->center;
    }

    // Queries

    template<int n>
    bool SphereN<n>::contains(const falg::Matrix<n, 1>& p) const {
        return r * r >= (p - this->center).sqNorm();
    }
};
