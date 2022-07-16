
#include <limits>

#include <HGraf/improc.hpp>

#include <FlatAlg.hpp>

namespace hg {
/*
 * MDC - MaxDiffComparator, take the difference between two values of some type
 */
    template <typename T> class MDC { public: static float diff(const T& t0, const T& t1); };

    template <>
    class MDC<float> {
    public: static float diff(const float& f0, const float& f1) { return std::abs(f0 - f1); }
    };


    template <>
    class MDC<double> {
    public:
        static float diff(const double &d0, const double &d1) { return std::abs(d0 - d1); }
    };

    template <>
    class MDC<unsigned char> {
    public:
        static float diff(const unsigned char &uc1, const unsigned char &uc2) { return std::abs((int)uc1 - (int)uc2); }
    };

    template <int n>
    class MDC<falg::Vector<n>> {
    public: static float diff(const falg::Vector<n>& v0, const falg::Vector<n>& v1) {
        float msum = - std::numeric_limits<float>::infinity();
        for (int i = 0; i < n; i++) {
            msum = std::max(msum, (float)std::abs(v0[i] - v1[i]));
        }

        return msum;
    }
    };


    template <typename T, std::size_t n>
    class MDC<std::array<T, n>> {
    public: static float diff(const std::array<T, n>& a0, const std::array<T, n>& a1) {
        float msum =  - std::numeric_limits<float>::infinity();
        for (int i = 0; i < a0.size(); i++) {
            msum = std::max(msum, (float)std::abs(a0[i] - a1[i]));
        }

        return msum;
    }
    };

    template<typename T>
    class ImageMaxDiffComparator: public flawed::FlComparator<hg::Image<T>> {
    public:
        virtual float compare(const hg::Image<T>& im0, const hg::Image<T>& im1) {
            fl_assert_eq(im0.getWidth(), im1.getWidth());
            fl_assert_eq(im0.getHeight(), im1.getHeight());

            float msum = - std::numeric_limits<float>::infinity();
            for (int i = 0; i < im0.getHeight(); i++) {
                for (int j = 0; j < im0.getWidth(); j++) {
                    msum = std::max(msum, MDC<T>::diff(im0.getPixel(j, i), im1.getPixel(j, i)));
                }
            }

            return msum;
        }
    };

    /**
     * SSIM
     */

    template<typename T>
    class ImageSSIMComparator: public flawed::FlComparator<hg::Image<T>> {
    public:
        virtual float compare(const hg::Image<T>& im0, const hg::Image<T>& im1) {
            fl_assert_eq(im0.getWidth(), im1.getWidth());
            fl_assert_eq(im0.getHeight(), im1.getHeight());

            return 1;
        }
    };
};
