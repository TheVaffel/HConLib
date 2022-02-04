#pragma once

#include "./improc.hpp"
#include <algorithm>

#include <flawed_assert.hpp>

namespace hg {
    template<unsigned int kernel_size, typename T = float>
    class ImageFilter {
        std::array<std::array<T, kernel_size>, kernel_size> data;

        ImageFilter();

        static ImageFilter<kernel_size, T> constructFilled(const T& value);

    public:
        ImageFilter(const std::initializer_list<std::initializer_list<T>>& init);

        static ImageFilter<kernel_size, T> zeros();
        static ImageFilter<kernel_size, T> ones();

        static ImageFilter<kernel_size, T> gaussianApprox();

        std::array<T, kernel_size>& operator[](unsigned int i);
        const std::array<T, kernel_size>& operator[](unsigned int i) const;

    };

    template<int s, typename T>
    ImageFilter<s, T> operator*(const ImageFilter<s, T>& in, T f);

    template<int s, typename T>
    ImageFilter<s, T> operator*(T f, const ImageFilter<s, T>& in);
};


/*
 * Implementation
 */
namespace hg {
    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T>::ImageFilter() { }

    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T>::ImageFilter(const std::initializer_list<std::initializer_list<T>>& init) {
        fl_assert_eq(init.size(), kernel_size);

        for (unsigned int i = 0; i < kernel_size; i++) {
            fl_assert_eq(std::begin(init)->size(), kernel_size);
        }

        for (unsigned int i = 0; i < kernel_size; i++) {
            const std::initializer_list<T>& thisRow = std::begin(init)[i];
            for (unsigned int j = 0; j < kernel_size; j++) {
                this->data[i][j] = std::begin(thisRow)[j];
            }
        }
    }

    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T> ImageFilter<kernel_size, T>::constructFilled(const T& value) {
        ImageFilter<kernel_size, T> out;

        for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
                out[i][j] = value;
            }
        }

        return out;
    }

    template<unsigned int kernel_size, typename T>
    std::array<T, kernel_size>& ImageFilter<kernel_size, T>::operator[](unsigned int i) {
        return data[i];
    }

    template<unsigned int kernel_size, typename T>
    const std::array<T, kernel_size>& ImageFilter<kernel_size, T>::operator[](unsigned int i) const {
        return data[i];
    }

    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T> ImageFilter<kernel_size, T>::zeros() {
        return ImageFilter<kernel_size, T>::constructFilled(0);
    }

    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T> ImageFilter<kernel_size, T>::ones() {
        return ImageFilter<kernel_size, T>::constructFilled(1);
    }

    template<unsigned int s>
    static std::array<long long, s> _get_pascal_row() {
        std::array<long long, s> arr;
        long long num = 1;
        for (int i = 0; i < s; i++) {
            arr[i] = num;

            num *= (s - i - 1);
            num /= (i + 1);
        }

        return arr;
    }

    template<unsigned int s, typename T>
    ImageFilter<s, T> ImageFilter<s, T>::gaussianApprox() {
        ImageFilter<s, T> res;

        std::array<long long, s> pascal = _get_pascal_row<s>();

        double sum = 0;
        for (int i = 0; i < s; i++) {
            for (int j = 0; j < s; j++) {
                double mul = pascal[i] * pascal[j];
                sum += mul;
                res[i][j] = mul;
            }
        }

        for (int i = 0; i < s; i++) {
            for (int j = 0; j < s; j++) {
                res[i][j] /= sum;
            }
        }

        return res;
    }


    template<unsigned int s, typename T>
    ImageFilter<s, T> operator*(const ImageFilter<s, T>& in, T f) {
        ImageFilter<s, T> out = ImageFilter<s, T>::zeros();
        for (int i = 0; i < s; i++) {
            for(int j = 0; j < s; j++) {
                out[i][j] = in[i][j] * f;
            }
        }

        return out;
    }

    template<unsigned int s, typename T>
    ImageFilter<s, T> operator*(T f, const ImageFilter<s, T>& in) {
        return in * f;
    }


    template<typename T, unsigned int s, typename FT = float>
    std::unique_ptr<Image<T>> filterImage(const Image<T>& im,
                                          const ImageFilter<s, FT>& kernel) {
        static_assert(s % 2 == 1, "[improc] Kernel dimensions must be odd");

        unsigned int w = im.getWidth();
        unsigned int h = im.getHeight();

        std::unique_ptr<Image<T>> res = std::make_unique<Image<T>>(w, h,
                                                                   im.getByteStride());
        unsigned int sh = s / 2;
        for (unsigned int i = 0; i < h; i++) {
            for (unsigned int j = 0; j < w; j++) {
                unsigned int minkx = std::max(0, (int)sh - (int)j);
                unsigned int minky = std::max(0, (int)sh - (int)i);

                unsigned int maxkx = std::min(s, w - j + sh);
                unsigned int maxky = std::min(s, h - i + sh);

                T sum = T();

                for (unsigned int k = minky; k < maxky; k++) {
                    for (unsigned int l = minkx; l < maxkx; l++) {
                        T pixel = im.getPixel(j + l - sh, i + k - sh);
                        sum = sum + pixel* kernel[k][l];
                    }
                }

                res->setPixel(j, i, sum);
            }
        }

        return res;
    }
};
