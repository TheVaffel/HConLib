#pragma once

#include <FlatAlg.hpp>

namespace hg {

    /*
     * RangeN - N-dimensional range - rectangle in 2D, box in 3D
     */

    template<int n>
    struct RangeN {
        falg::Matrix<n, 1> min, max;

        RangeN();
        RangeN(const falg::Matrix<n, 1>& minp, const falg::Matrix<n, 1>& maxp);

        static RangeN<n> fromMiddleRange(const falg::Matrix<n, 1>& middle, const falg::Matrix<n, 1>& range);

        falg::Matrix<n, 1> getMin() const;
        falg::Matrix<n, 1> getMax() const;

        const falg::Matrix<n, 1>& getMiddle() const;
        const falg::Matrix<n, 1>& getRange() const;
        falg::Matrix<n, 1> getDims() const;

        falg::Matrix<n, 1>& getMin();
        falg::Matrix<n, 1>& getMax();

        void add(const falg::Matrix<n, 1>& p);
        bool contains(const falg::Matrix<n, 1>& p) const;
    };


    /*
     * SphereN - N-dimensional sphere - circle in 2D, sphere in 3D
     */

    template<int n>
    struct SphereN {
        falg::Matrix<n, 1> center;
        float r;

        SphereN(const falg::Matrix<n, 1>& center, float r);
        bool contains(const falg::Matrix<n, 1>& p) const;

        const float& getRadius() const;
        float& getRadius();

        falg::Matrix<n, 1>& getCenter();
        const falg::Matrix<n, 1>& getCenter() const;
    };

    typedef RangeN<2> Range2;
    typedef RangeN<3> Range3;
    typedef Range2 Rectangle;
    typedef Range3 Box;

    typedef SphereN<2> Circle;
    typedef SphereN<3> Sphere;

};
